# Import the os module, for the os.walk function
import os
 
# Set the directory you want to start from
rootDir = '.\\tools'
for dirName, subdirList, fileList in os.walk(rootDir):
	print('Found directory: %s' % dirName)
	for fname in fileList:
		if fname.endswith('.c') or fname.endswith('.m') or fname.endswith('.cpp') or fname.endswith('.h') or fname.endswith('.inl'):
			print('\t%s' % fname)
			with open(dirName + '\\' + fname + '~', 'w') as outfile:
				outfile.write('/*\n')
				outfile.write('================================================================================================\n')
				outfile.write('CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION\n')
				outfile.write('Copyright 2017 Doctor Entertainment AB. All Rights Reserved.\n')
				outfile.write('================================================================================================\n')
				outfile.write('*/\n')
				with open(dirName + '\\' + fname) as infile:
					for line in infile:
						outfile.write(line)
			os.remove(dirName + '\\' + fname)
			os.rename(dirName + '\\' + fname + '~', dirName + '\\' + fname)
