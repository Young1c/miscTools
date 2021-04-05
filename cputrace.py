#!/usr/bin/env python
#
# cputrace.py   Simple program for counting the cpus of a process runned on
#
# USAGE: cputrace.py [-h] [-p PID] [-f FREQ] ...
#
# Copyright Yicong Yang
# Author: Yicong Yang <young.yicong@outlook.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

import argparse
import atexit
import multiprocessing
import os, sys, signal
import threading
import time

nr_cpus = multiprocessing.cpu_count()
stats = [0] * nr_cpus
start_time = float()
threadsNumber = list()
threadsPath = list()
threadsStat = dict()

# Init the parser
has_pid = False
has_program = False

parser = argparse.ArgumentParser(description = "Simple program for counting the cpus of a process running on")
parser.add_argument('-p', '--pid', type = int, help = "target pid to trace")
parser.add_argument('-f', '--freq', type = int, help = "sampling frequency", default = 50)
parser.add_argument('program', nargs = argparse.REMAINDER)
args = parser.parse_args()

if args.freq:
	sampling_interval = 1 / args.freq

if args.pid:
	target_pid = args.pid

if args.program:
	exec_path = args.program

if not args.pid:
	if not args.program:
		has_program = False
	else:
		has_program = True
else:
	has_pid = True

endEvent = threading.Event()

def getCpuOfPid(thread: int):
	try:
		with open(threadsPath[thread]) as file:
			thread_stat = file.readline()
			cpu_on = thread_stat.split(' ')[38]
			threadsStat[threadsNumber[thread]][int(cpu_on)] += 1
			file.close()
	except:
		endEvent.set()

def updateThreadStat():
	try:
		currentThreads = [ int(i) for i in os.listdir("/proc/%d/task" % (target_pid)) if i != '.' and i != '..' ]
	except:
		endEvent.set()
		return

	for id in currentThreads:
		if not id in threadsNumber:
			threadsNumber.append(id)
			threadsPath.append("/proc/%d/task/%d/stat" % (target_pid, id))
			threadsStat[id] = [0] * nr_cpus

def statistic():
	while not endEvent.isSet():
		time.sleep(sampling_interval)
		updateThreadStat()

		for thread in range(len(threadsNumber)):
			if (endEvent.isSet()):
				break
			getCpuOfPid(thread)

statisticThread = threading.Thread(target=statistic)

def exitShow():
	print("duration: %f secs, total threads: %d, sampling interval: %f secs" % (time.time() - start_time, len(threadsNumber), sampling_interval))

	if not endEvent.isSet():
		endEvent.set()
		time.sleep(0.1)
		if has_program:
			os.kill(target_pid, signal.SIGKILL)

	statisticThread.join()

	for i in threadsNumber:
		stats = threadsStat[i]

		print("threads: %d" % (i))
		for cpu in range(nr_cpus):
			print("cpu %d: %d" % (cpu, stats[cpu]), end = ' ')
		print()

def main():
	atexit.register(exitShow)

	global start_time
	global target_pid

	mainprocess = True

	if not has_pid:
		if not has_program:
			atexit.unregister(exitShow)
			parser.print_help()
			return -1
		else:
			global target_pid
			target_pid = os.fork()
			if target_pid == 0:
				mainprocess = False
				atexit.unregister(exitShow)
				print("exec program pid %d" % (os.getpid()))
				os.execvp(exec_path[0], exec_path)

	if mainprocess:
		start_time = time.time()
		statisticThread.start()

		if has_program:
			try:
				os.wait()
			except:
				# just catch the keyboard interrupt
				pass
			finally:
				endEvent.set()

	return 0

if __name__ == '__main__':
	main()
