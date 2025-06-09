#!/usr/bin/env ruby

# Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: MIT

require 'pp'
require 'open3'
require 'optparse'

$res = 0
$verbose = true

def run cmd
  if $verbose
    STDOUT.print "RUN: #{cmd}"
    STDOUT.flush
  end

  a = Time.now
  o, e, s = Open3.capture3(cmd)
  b = Time.now

  if $verbose
    STDOUT.print " -> #{s} in %1.3fs\n" % [b - a]
    STDOUT.flush
  end

  if s.to_i != 0 or e.size > 0
    raise "CMD: #{cmd} status: #{s.to_i}, std-err: #{e}"
  end

  return o, e, s
end

def sys cmd
  if $verbose
    puts "RUN: #{cmd}"
  end

  a = Time.now
  system cmd
  b = Time.now

  if $verbose
    puts "RUN-Done: #{cmd} -> #{$?} in %1.3fs\n" % [b - a]
    STDOUT.flush
  end

  if $?.to_i != 0
    raise "CMD: #{cmd} status: #{$?.to_i}"
  end
end

def try cmd
  begin
    run cmd
  rescue
    puts "CMD: #{cmd} failed (but will continue)"
    $res = 1
  end
end

def cd path
  puts "cd #{path}  (was: #{Dir.pwd})" if $verbose
  Dir.chdir(path)
end

$top = File.dirname(File.dirname(File.expand_path(__FILE__)))
cd $top

git_sha = %x(git rev-parse --short HEAD).chop
git_sha_long = %x(git rev-parse HEAD).chop
begin
    git_id = %x(git describe --tags --long).chop
rescue
    git_id = git_sha
end
if ENV['BRANCH_NAME']
    git_branch = ENV['BRANCH_NAME']
else
    git_branch = %x(git symbolic-ref --short -q HEAD).chop
end

report_name = "static-analysis-report-#{git_id}-#{git_branch}"

if File.exist? "#{report_name}"
    run "rm -rf #{report_name}"
end

sys "mkdir #{report_name}"

sys "cp -r static_analysis_reports/* #{report_name}"

cmd = [".cmake/artifactory-ci-upload"]
cmd << "-vvv"
cmd << "--dep-file .cmake/deps-bsp.json"
cmd << "--dep-file .cmake/deps-docker.json"
cmd << "--dep-file .cmake/deps-toolchain.json"
cmd << report_name

sys cmd.join(" ")

run "cp -r #{report_name} images/." if File.exist? "./images"

run "rm -rf #{report_name}"


out_name = "mepa-#{git_id}-#{git_branch}"

raise "No ws folder" if not File.exist? "./ws"

if File.exist? "#{out_name}"
    run "rm -rf #{out_name}"
end
sys "cp -r ws #{out_name}"
sys "mkdir #{out_name}/bin"
sys "tar -C #{out_name}/bin -f arm64.tar -x"
run "tar -czvf #{out_name}.tar.gz #{out_name}"

if File.exist? "./images"
  sys "cp #{out_name}/bin/arm64/mepa_demo/*.itb images/."
  sys "cp #{out_name}/bin/arm64/mepa_demo/*.ext4.gz images/."
end

run "rm -rf #{out_name}"

cmd = [".cmake/artifactory-ci-upload"]
cmd << "-vvv"
cmd << "--dep-file .cmake/deps-bsp.json"
cmd << "--dep-file .cmake/deps-docker.json"
cmd << "--dep-file .cmake/deps-toolchain.json"
cmd << "#{out_name}.tar.gz"
sys cmd.join(" ")

run "cp #{out_name}.tar.gz images/." if File.exist? "./images"

exit $res

