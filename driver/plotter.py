import matplotlib.pyplot as plt
import pandas as pd
import re

re_perceptron = re.compile('(perceptron:\(.+,.+\))')
re_accuracy = re.compile('(bpred_perceptron.bpred_dir_rate)[ ]*[0-9][\.0-9]*')
re_accuracy_value = re.compile('[0-9][\.0-9]*')
re_perc_vars = re.compile('\(.+,.+\)')
data = {'global':[], 'local':[], 'accuracy':[]}

with open("ece621-python.data", "r") as datafile:
    for cnt, line in enumerate(datafile):
        if line=="EXPT_BEGIN" or line=="EXPT_END":
            assert(len(data['global']) == len(data['local']) == len(data['accuracy']))
        elif re_perceptron.match(line):
            raw_vars = re_perc_vars.findall(line)
            vars = raw_vars[0][1:-1].split(',')
            data['global'].append(int(vars[0]))
            data['local'].append(int(vars[1]))
        elif re_accuracy.match(line):
            acc = re_accuracy_value.findall(line)[0]
            data['accuracy'].append(float(acc))
            
