import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy import stats

#np.array()
hwb_list = [1,2,4,8,16,32,64,128,256,512]
perc_acc_addr = []
perc_acc_dir = []
tlev_acc_addr = []
tlev_acc_dir = []

df = pd.DataFrame.from_csv("perceptvs2lev.csv")
for hwb in hwb_list:
    perc_budget = df.loc[(df['Predictor'] == 'perceptron') & (df['Hardware Budget'] == hwb)]
    tlev_budget = df.loc[(df['Predictor'] == '2lev') & (df['Hardware Budget'] == hwb)]
    perc_acc_addr.append(stats.hmean((1-perc_budget['Accuracy Addr'].values))*100)
    perc_acc_dir.append(stats.hmean((1-perc_budget['Accuracy Dir'].values)*100))
    tlev_acc_addr.append(stats.hmean((1-tlev_budget['Accuracy Addr'].values)*100))
    tlev_acc_dir.append(stats.hmean((1-tlev_budget['Accuracy Dir'].values)*100))

#breakpoint()
fig = plt.figure()
fig.suptitle("Accuracy of predictors as a function of hardware budget")
ax1 = fig.add_subplot(211)
ax1.plot(hwb_list, perc_acc_addr)
ax1.plot(hwb_list, perc_acc_dir)
ax1.set_xscale("log", basex=2)
ax1.set_xlim(1,512)
ax1.set_title("Perceptron")
ax1.set_xlabel("Hardware budget (Kilobytes)")
ax1.set_ylabel("Misprediction rate (%)")
ax1.set_ylim(0)
ax2 = fig.add_subplot(212)
ax2.plot(hwb_list, tlev_acc_addr)
ax2.plot(hwb_list, tlev_acc_dir)
ax2.set_xscale("log", basex=2)
ax2.set_xlim(1,512)
ax2.set_title("Two level predictor")
ax2.set_ylim(0)
ax2.set_ylabel("Misprediction rate (%)")
ax2.set_xlabel("Hardware budget (Kilobytes)")
plt.show()
