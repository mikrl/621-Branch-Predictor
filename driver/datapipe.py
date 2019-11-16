import matplotlib.pyplot as plt
import pandas as pd
import re

re_expt = re.compile('(HW\:)([0-9]+)(\-)(.*)(\-)(.*)')
re_perceptron = re.compile('(perceptron:\(.+,.+\))')
re_accuracy_dir_perceptron = re.compile('(bpred_perceptron.bpred_dir_rate)[ ]*[0-9][\.0-9]*')
re_accuracy_dir_2lev = re.compile('(bpred_2lev.bpred_dir_rate)[ ]*[0-9][\.0-9]*')
re_accuracy_addr_perceptron = re.compile('(bpred_perceptron.bpred_addr_rate)[ ]*[0-9][\.0-9]*')
re_accuracy_addr_2lev = re.compile('(bpred_2lev.bpred_addr_rate)[ ]*[0-9][\.0-9]*')
re_accuracy_value = re.compile('[0-9][\.0-9]+')
re_perc_vars = re.compile('\(.+,.+\)')
data = {'Hardware Budget':[], 'Predictor':[], 'Benchmark':[], 'Accuracy Addr':[], 'Accuracy Dir':[]}

with open("ece621-perceptvs2lev-python.data", "r") as datafile:
    for cnt, line in enumerate(datafile):
        if (line is"EXPT_BEGIN\n") or (line is "EXPT_END\n"):
            assert(len(data['Hardware Budget']) == len(data['Predictor']) == len(data['Benchmark']) == max(len(data['Accuracy Addr']), len(data['Accuracy Dir'])))
        elif re_expt.match(line):
            data_cfg = re_expt.split(line)
            data['Hardware Budget'].append(data_cfg[2])
            data['Predictor'].append(data_cfg[4])
            data['Benchmark'].append(data_cfg[6])
        elif (re_accuracy_addr_perceptron.match(line) or re_accuracy_addr_2lev.match(line)):
            #breakpoint()
            acc = re_accuracy_value.findall(line)[0]
            data['Accuracy Addr'].append(float(acc))
        elif  (re_accuracy_dir_perceptron.match(line) or re_accuracy_dir_2lev.match(line)):
            #breakpoint()
            acc = re_accuracy_value.findall(line)[0]
            data['Accuracy Dir'].append(float(acc))

df = pd.DataFrame.from_dict(data)
df.to_csv("./perceptvs2lev.csv")
