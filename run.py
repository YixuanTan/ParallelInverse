from subprocess import Popen, PIPE
import numpy as np
import matplotlib.pyplot as plt

interval = 0.1
first = 0.0
last = 10.0
num = (int)((last - first) / interval)
count = 1;
for k in xrange(0, num + 1):
	print 'iter ', k
	start = 0.0 + interval * k;
	end = start + 1.0
	process = Popen('./driver ' + str(start) + ' ' + str(end), stdout=PIPE, stderr=PIPE, shell=True)
	stdout, stderr = process.communicate()
	#print type(stdout) 
	profile = np.reshape(np.fromstring(stdout, dtype = float, sep=' '), (-1,2))
	print 'profile is \n', profile.shape
	plt.plot(profile[:,0], profile[:,1], "-o")
	plt.ylim([400, 800])
	plt.xlim([0, 10])
	plt.hold(True)
	plt.axvline(x=start, color='r') 
	plt.hold(True)
	plt.axvline(x=end, color='r') 
	plt.savefig(str(count) + '.png')
	plt.gcf().clear()
	count = count + 1



