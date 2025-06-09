#!/usr/bin/env ruby
# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

require 'pp'
require 'pty'
require 'json'
require 'open3'
require 'thread'
require 'optparse'
require 'pathname'
require 'shellwords'
require 'fileutils'
require 'csv'

$opt_path = nil
$opt_threads = (%x{grep -c ^processor /proc/cpuinfo}.to_i * 1.5).to_i
$top = File.dirname(File.dirname(File.expand_path(__FILE__)))
$cov_bin = "/opt/coverity/analysis/bin"

opt_parser = OptionParser.new do |opts|
  opts.banner = """Usage: mepa-coverity [options] [<OUTPUT-FOLDER>]

Options:"""
  opts.on("-t", "--target TARGET", "Target to run coverity on") do |t|
    $opt_target = t
  end

  opts.on("-v", "--verbose", "Be more verbose") do
    $opt_verbose = 1
  end

  opts.on("-h", "--help", "Show this message") do
    puts opts
    exit
  end
end

opt_parser.parse!(ARGV)

def run cmd
  puts cmd if $opt_verbose
  o, e, s = Open3.capture3(cmd)
  if $?.to_i != 0
    raise "CMD: #{cmd} failed!"
  end
end

if $opt_target.nil?
  puts "Missing option: -t TARGET"
  exit -1
end

if ARGV[0].nil?
   puts "Output Folder name missing"
   exit -1
end

$opt_path = "#{$top}/build-#{$opt_target}"

def coverity_derive_config cmd
  conf = []
  skip_next = false
  cmd.split(" ").each do |e|
    if not skip_next and (e == "-o" or e == "-c" or e == "-include" or e == "-x")
      skip_next = true
    elsif skip_next
      skip_next = false
    elsif e =~ /^-(I|D|O|W|g)/
    else
      conf << e
    end
  end

  type = "unknown"
  case $opt_target
  when "arm64"
    compiler = "aarch64-linux-gnu-gcc"
    type = "gcc"
  when "arm"
    compiler = "arm-cortex_a8-linux-gnu-gcc"
    type = "gcc"
  end
  "#{$cov_bin}/cov-configure --config /opt/coverity/analysis/config/coverity_config.xml --compiler #{conf[0]} --comptype #{type} -- #{conf.join(" ")} > config.log 2>&1"
end

def coverity_output_message r
  if r[:status] == 0
    puts "OK: #{r[:file]}"
    $cnt_ok += 1
  else
    pp r
    $cnt_err += 1
  end
end

file_path = "#{$top}/compile_commands.json"
json_text = File.read(file_path)
$compile_command_records = JSON.parse(json_text)

$compile_command_records = $compile_command_records.map do |entry|
 {
    file: entry["file"],
    directory: entry["directory"],
    command: entry["command"]
 }
end

$cnt_ok = 0
$cnt_err = 0
$confs = []
work_q = Queue.new
mutex = Mutex.new

$compile_command_records.each do |e|
  f = e[:file]
  cmd = e[:command]
  
  next if f.nil? || cmd.nil?  # Skip entries without file or command
  ff = Pathname.new(f).relative_path_from(Pathname.new(Dir.pwd)).to_s
  
  next unless ff.include?("mepa/")
  next if ff.end_with?(".o")
  next if ff.end_with?(".S")

  record = {:file => ff }
  $confs << coverity_derive_config(cmd)
  record[:cmd] = "#{$cov_bin}/cov-translate --emit-complementary-info --config /opt/coverity/analysis/config/coverity_config.xml --dir #{$opt_path}/cov_output #{e[:command]}"
  work_q.push record
end

$confs.uniq!
run "rm -rf #{$opt_path}/cov_output"

$confs.each do |c|
  run c
end

system "cat config.log"

workers = (0...$opt_threads).map do
  Thread.new do
    begin
      while record = work_q.pop(true)
        cmd = record[:cmd]
        o = ""
        e = ""
        s = 0

        time_start  = Time.now
        o, e, s = Open3.capture3(cmd)
        time_end = Time.now
        time_spend = time_end - time_start
        record[:stdout] = o
        record[:stderr] = e
        record[:status] = s
        record[:timespend] = time_spend

        mutex.synchronize {
          coverity_output_message record
        }

      end
    rescue ThreadError
    end
  end
end

workers.map(&:join)
puts "Total: #{$cnt_ok + $cnt_err}, OK: #{$cnt_ok}, Error: #{$cnt_err}"


coding_standards = {
  certc: {
    config: "/opt/coverity/analysis/config/coding-standards/cert-c/cert-c-all.config",
    html:   "#{$opt_path}/cov_output/html_output_certc",
    json:   "#{$opt_path}/cov_output/coverity_certc.json",
    csv:    "#{$opt_path}/cov_output/coverity_certc.csv"
  },

  misra2023_mandatory_req: {
    config: "/opt/coverity/analysis/config/coding-standards/misrac2023/misrac2023-mandatory-required.config",
    html:   "#{$opt_path}/cov_output/html_output_misra2023_mandatory_req",
    json:   "#{$opt_path}/cov_output/coverity_misra2023_mandatory_req.json",
    csv:    "#{$opt_path}/cov_output/coverity_misra2023_mandatory_req.csv"
  },
}


coding_standards.each do |key, cfg|
  puts "Analyzing for #{key} coding standard....."

  # Run cov-analyze
  system "/opt/coverity/analysis/bin/cov-analyze --config /opt/coverity/analysis/config/coverity_config.xml --dir #{$opt_path}/cov_output -s #{Dir.pwd} --coding-standard-config #{cfg[:config]}"

  # Generate JSON report
  system "/opt/coverity/analysis/bin/cov-format-errors --dir #{$opt_path}/cov_output  --json-output-v10 #{cfg[:json]}"

  # Generate HTML report
  system "rm -rf #{cfg[:html]}"
  system "/opt/coverity/analysis/bin/cov-format-errors --dir #{$opt_path}/cov_output  --html-output #{cfg[:html]}"

  data = JSON.parse(File.read("#{cfg[:json]}"))

  if data["issues"].empty?
    puts "No violations found for #{key} coding standard"
  end

  CSV.open("#{cfg[:csv]}", "wb") do |csv|
    csv << ["Checker Name", "File Path", "Line Number", "Function", "Impact"]
    data["issues"].each do |issue|
      checker_name = issue["checkerName"] || ""
      filepath = issue["strippedMainEventFilePathname"] || ""
      line_number = issue["mainEventLineNumber"] || ""
      function = issue["functionDisplayName"] || ""
      impact = issue.dig("checkerProperties", "impact") || ""

      csv << [checker_name, filepath, line_number, function, impact]
      # Increment error count only for High or Medium impact
      if impact.casecmp?("High") || impact.casecmp?("Medium")
        $cnt_err += 1
      end
    end
  end
end

puts "Total Violations: #{$cnt_err}"

output_dir = ARGV[0]

FileUtils.mkdir_p(output_dir)
puts "Created a output directory #{output_dir}"

csv_files = coding_standards.map { |_, cfg| cfg[:csv] }

csv_files.each do |file|
  if File.exist?(file)
    FileUtils.cp(file, output_dir)
  else
    puts "Warning: CSV file not found - #{file}"
  end
end

if $? != 0
  $cnt_err += 1
end

if $cnt_err > 0
  exit 1
else
  exit 0
end
