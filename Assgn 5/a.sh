#!/bin/bash

# Get the list of semaphores owned by the current user
semaphores=$(ipcs -s | grep "$(whoami)" | awk '{print $2}')

# Check if there are any semaphores to remove
if [ -z "$semaphores" ]; then
    echo "No semaphores found."
else
    # Loop through each semaphore and remove it
    for semaphore in $semaphores; do
        ipcrm -s $semaphore
        echo "Removed semaphore $semaphore"
    done
fi

# Get the list of shared memory segments owned by the current user
segments=$(ipcs -m | grep "$(whoami)" | awk '{print $2}')

# Check if there are any segments to remove
if [ -z "$segments" ]; then
    echo "No shared memory segments found."
else
    # Loop through each segment and remove it
    for segment in $segments; do
        ipcrm -m $segment
        echo "Removed shared memory segment $segment"
    done
fi
