.PONY: install test img_proc_test

COMP=mpicc
NUM_THREADS=2

BUILD_FOLDER=/home/erick/google_drive/TEC/SistemasOperativos/Proyectos/Proyecto3/HPC-Cluster/shared
TEST_FILE=test

HOSTS=worker1,manager

install:
	# Update
	sudo apt update
	# Install the dependencies for the project.
	sudo apt-get install -y mpi libopenmpi-dev openssh-server nfs-kernel-server \
	nfs-common



test:
	# If the folder doesnt exists create it
	@if [ ! -d ${BUILD_DIR} ]; then \
		mkdir ${BUILD_DIR}; \
	fi
	# Compile the code
	@${COMP} ${TEST_FILE}.c -o ${BUILD_FOLDER}/${TEST_FILE}
	# Update the files in the shared folder
	sudo exportfs -a
	# Run it on the cluster
	@mpirun -n ${NUM_THREADS} -host ${HOSTS} ${BUILD_FOLDER}/${TEST_FILE}

img_proc_test:
	# If the folder doesnt exists create it
	@if [ ! -d ${BUILD_DIR} ]; then \
		mkdir ${BUILD_DIR}; \
	fi
	# Compile the code
	@${COMP} img_proc.c -o ${BUILD_FOLDER}/img_proc -lm -lpng
