#!/bin/bash

# Define the owner user of everuthing
OWN_USR=erick

# Hosts file
HOSTS=/etc/hosts
# Exports file
EXPORTS=/etc/exports
# File system table
FSTAB=/etc/fstab

# Get current dir
CURRENT_DIR=$(pwd)

# Manager shared dir
MAN_SHR_DIR=/home/erick/google_drive/TEC/SistemasOperativos/Proyectos/Proyecto3/HPC-Cluster/shared

# Shared directory
SHARED_DIR=$CURRENT_DIR/shared

# Set the target manager name and ip
TARGET=$1
IP=$2
KEY=key_$TARGET

# Check if the file exists
if ! [ -f "$HOSTS" ]; then
    # If it doesnt, exit
    echo "Target file $HOSTS not found."
    exit 
fi

# Check if the target word is missing from the hosts file
if grep -q $TARGET "$HOSTS"; then
    # If it exists then do nothing
    echo "$TARGET already added in $HOSTS file."
    exit

else    
   # If missing then add the ip and the missing word
    sudo echo -e "$IP\t$TARGET" >> $HOSTS
fi

# Generate a key 
ssh-keygen -t rsa -f ./$KEY
# Make erick the default owner of the keys
sudo chown -v $OWN_USR ./$KEY
sudo chown -v $OWN_USR ./$KEY.pub
# Asociate it with the target
sudo ssh-copy-id -i ./$KEY $OWN_USR@$TARGET

# Add the private key 
#chmod 400 ./$KEY
eval `ssh-agent`
ssh-add ./$KEY

# Create shared directory if missing
if ! [ -d "$SHARED_DIR" ]; then
    mkdir $SHARED_DIR
fi



# Proceed with the nfs cofiguration
# Check if we are configuring the manager
if echo $TARGET | grep -q "worker"; then
    # Export the shared folder
    #    path/to/dir target_ip(permissions)
    echo "$SHARED_DIR *(rw,sync,no_root_squash,no_subtree_check)" >> $EXPORTS
    # Update changes in exports file
    exportfs -a
    # Re-start the nfs server
    #service nfs-kernel-server restart

# Check if we are configuring a worker
elif echo $TARGET | grep -q "manager"; then
    # Mount the deefault manager shared directory
    sudo mount -t nfs manager:$MAN_SHR_DIR $SHARED_DIR
    # Check everything was mounted correctly
    df -h
    # Set the mount to be automatic so we dont have to do it every system reboot
    echo "manager:$MAN_SHR_DIR /home/erick/HPC-Cluster/shared nfs" >> $FSTAB
fi