import os

print('Building executables...')

os.system('make no_parallism_lodepng')
os.system('make cuda_lodepng')
os.system('mkdir -p export/')

print('Running tests...')

def run_opt(opt, filename):
	file_in = 'examples/' + filename
	file_out = 'export/perf_' + opt + '_' + filename

	print('+++ WITHOUT CUDA +++')
	os.system('bin/image_modifier_no_parallism %s %s %s' % (opt, file_in, file_out))
	print('+++ CUDA +++')
	os.system('bin/image_modifier_cuda %s %s %s' % (opt, file_in, file_out))

files = os.listdir('examples/')

for image in files:
	head, tail = os.path.split(image)
	run_opt('grey', tail)
	run_opt('blur', tail)
	run_opt('hsv', tail)