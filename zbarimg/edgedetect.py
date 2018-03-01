#!/usr/bin/env python
li = [1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1]
li_1st = []
li_2ed = []

def differential(list):
	diff_list = []
	for i in range(1,len(list)):
		temp = list[i] - list[i-1]
		diff_list.append(temp)
	return diff_list

li_1st = differential(li)
li_2ed = differential(li_1st)
print li
print li_1st
print li_2ed