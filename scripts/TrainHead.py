import numpy as np
import matplotlib.pyplot as plt
import math
import sys
import re
import cPickle
from sklearn.ensemble import RandomForestRegressor

def wrapData(fname):
	lineCounter = 0
	dataList = []
	with open(fname) as f:
		content = f.readlines()
		for data in content:
			lineCounter += 1
			elements = re.findall(r"[\d\.-]+", data)
			wrapdata = []
			for element in elements:
				if len(element) > 4:
					wrapdata.append(float(element))
				if len(wrapdata) == 3:
					break
			dataList.append(wrapdata)
	print "Wrap " + str(len(dataList)) + " of data successfully"
	return dataList

def splitData(dataList, windowSize):
	X = []
	Y = []
	head = 0
	dataSize = len(dataList)
	flatSize = 3 * windowSize
	while (head + windowSize) < (dataSize - 1):
		X.append( np.reshape(dataList[head: head+windowSize], flatSize) )
		Y.append( dataList[head + windowSize][:3] )
		head += 1
	print "Split data successfully with " + str(len(X)) + " of samples"
	return (X, Y)

def scorePrediction(testResult, testY):
	totalErr = [0., 0., 0.]
	dataSize = len(testY)
	currentLoc = [0., 0., 0.]
	groundTruthLoc = [0., 0., 0.]
	for i in range(0, dataSize):
		for t in range(0, 3):
			currentLoc[t] += testResult[i][t]
			groundTruthLoc[t] += testY[i][t]
			if t == 1: # range from -PI/2 ~ PI/2
				currentLoc[t] = (currentLoc[t] - math.pi) if currentLoc[t] > math.pi/2 else currentLoc[t]
				currentLoc[t] = (currentLoc[t] + math.pi) if currentLoc[t] < -math.pi/2 else currentLoc[t]
				groundTruthLoc[t] = (groundTruthLoc[t] - math.pi) if groundTruthLoc[t] > math.pi/2 else groundTruthLoc[t]
				groundTruthLoc[t] = (groundTruthLoc[t] + math.pi) if groundTruthLoc[t] < -math.pi/2 else groundTruthLoc[t]
				diff = abs( currentLoc[t] - groundTruthLoc[t] )
				diff = (math.pi - diff) if diff > math.pi/2 else diff
				totalErr[t] += diff
			else: # range from -PI ~ PI
				currentLoc[t] = (currentLoc[t] - 2*math.pi) if currentLoc[t] > math.pi else currentLoc[t]
				currentLoc[t] = (currentLoc[t] + 2*math.pi) if currentLoc[t] < -math.pi else currentLoc[t]
				groundTruthLoc[t] = (groundTruthLoc[t] - 2*math.pi) if groundTruthLoc[t] > math.pi else groundTruthLoc[t]
				groundTruthLoc[t] = (groundTruthLoc[t] + 2*math.pi) if groundTruthLoc[t] < -math.pi else groundTruthLoc[t]
				diff = abs( currentLoc[t] - groundTruthLoc[t] )
				diff = (2*math.pi - diff) if diff > math.pi else diff
				totalErr[t] += diff
	return (totalErr[0] / dataSize, totalErr[1] / dataSize, totalErr[2] / dataSize)


if __name__ == '__main__': 
	argc = len(sys.argv)
	if argc < 3:
		print "Usage: python TrainHead.py [trainDataList] [testData]"
	
	trainDataListFileName = sys.argv[1]
	testDataFileName = sys.argv[2]

	trainX = []
	trainY = []

	with open(trainDataListFileName) as fn:
		filenames = fn.readlines()
		for filename in filenames:
			filename = filename.replace('\n', '')
			trainDataList = wrapData(filename)
			X, Y = splitData(trainDataList, 30)
			trainX += X
			trainY += Y

	testDataList = wrapData(testDataFileName)
	testX, testY = splitData(testDataList, 30)

	# Draw
	absoluteLoc = []
	currentLoc = [0., 0., 0.]
	for i in range(0, len(testY)):
		for t in range(0, 3):
			currentLoc[t] += testY[i][t]
			if currentLoc[t] < -math.pi:
				currentLoc[t] += math.pi
		absoluteLoc.append(list(currentLoc) )

	npTestY = np.array(absoluteLoc)
	plt.plot(npTestY[:,0], 'b-', npTestY[:,1], 'r-', npTestY[:,2], 'g-')
	plt.axis([len(testY)/50, len(testY)/10, -3.14, 3.14])
	plt.show()

	# Get simple file name
	trainDataListFileNameSimple = trainDataListFileName[(trainDataListFileName.rfind('/')+1):(trainDataListFileName.rfind('.'))]

	'''
	print "Start training..."
	clf = RandomForestRegressor(n_estimators=500).fit(trainX, trainY)

	print "Save classfier to file..."
	with open('clf_rf_' + trainDataListFileNameSimple + '.pkl', 'wb+') as fclf:
		cPickle.dump(clf, fclf)
		print "Save classifier succussfully"

	print "Start testing..."
	testResult = clf.predict(testX)
	
	'''
	# Test for stupid guessing
	testResult = []
	for i in range(0, len(testY)):
		testResult.append( [0., 0., 0.] )
	
	
	print "Done prediction"
	errX, errY, errZ = scorePrediction(testResult, testY)

	print "Error: " + str(errX) + ", " + str(errY) + ", " + str(errZ)
