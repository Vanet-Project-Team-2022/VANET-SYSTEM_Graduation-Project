#!/bin/bash
# ----------------------------------------------------------
# File: run.sh <config> [first-run last-run [cores]]
#
# Arguments:
#   1 - configuration name in omnetpp.ini file (mandatory)
#   2 - first run to execute; 0-based
#   3 - last run to execute
#   4 - number of cores; 2 by default
#
# NOTE: This script assumes a directory structure like this:
#    OMNET_WORKSPACE_PATH > PRJ_NAME > simulations > helloworld
#    where 'helloworld' is the working directory.
#    The directory names are got automatically, but if the
#    directory structure is different, it is necessary to modify
#    the environment variables PRJ_NAME and OMNET_WORKSPACE_PATH
#    by adding or removing some 'cd ..' below.
# ----------------------------------------------------------

if [ $# -eq 0 ] || [ $# -gt 4 ]; then
   echo "Error in arguments supplied; please, specify the following arguments:"
   echo "  1 - configuration name in omnetpp.ini file"
   echo "  2 - first run to execute; 0-based"
   echo "  3 - last run to execute"
   echo "  4 - number of cores; 2 by default"
   exit 1
fi

THISHOST=$(hostname -f)

# Executable name of the debugger
GDB=gdb

# Simulation name
SIM_NAME=${PWD##*/}
# Project name
PRJ_NAME=$(basename "$(dirname `cd .. && pwd`)")
# Workspace
OMNET_WORKSPACE_PATH=$(dirname `cd .. && cd .. && pwd`)
# Other paths
PATH_PRJ=$OMNET_WORKSPACE_PATH/$PRJ_NAME
PATH_PRJ_SRC=$PATH_PRJ/src
PATH_PRJ_SIMS=$PATH_PRJ/simulations
PATH_PRJ_SIM=$PATH_PRJ_SIMS/$SIM_NAME
PATH_PRJ_SIM_RESULTS=$PATH_PRJ_SIM/results
PATH_PRJ_VEINS=$OMNET_WORKSPACE_PATH/veins/src/veins

EXE=$PATH_PRJ_SRC/$PRJ_NAME
EXE_LN=$PATH_PRJ_SIM/$PRJ_NAME

# -------------------------------------
# Show paths before running the simulations
# -------------------------------------
function show_paths() {
   NUMRUNS=$(($LASTRUN - $FIRSTRUN + 1))
   echo "---------------------------"
   echo "Workspace    : $OMNET_WORKSPACE_PATH"
   echo "Veins source : $PATH_PRJ_VEINS"
   echo "Project path : $PATH_PRJ"
   echo "Results path : $PATH_PRJ_SIM_RESULTS"
   echo "Project name : $PRJ_NAME"
   echo "Simulation   : $SIM_NAME"
   echo "Configuration: $CONFIG"
   echo "Executable   : $EXE"
   if [ $NUMRUNS -gt 0 ]; then
      echo "Runs         : $FIRSTRUN..$LASTRUN --> total $NUMRUNS"
   fi
   echo "Cores        : $NUM_CORES"
   echo "---------------------------"
   echo ""
}

# -------------------------------------
# Ask confirmation to launch the simulations
# -------------------------------------
function confirm() {
   read -n1 -r -p "Press space to continue; any other key to abort ..." key
   if [ "$key" = '' ]; then
      # Space pressed; echo [$key] is empty when SPACE is pressed
      echo "Ok"
   else
      # Anything else pressed; echo [$key] not empty
      echo "Aborted by user!"
      exit 1
   fi
}

# -------------------------------------
# Make a symbolic links (it it does not exists)
# -------------------------------------
function make_link_exe() {
   # Check if the executable file exists
   if [ ! -f $EXE ]; then
      echo "ERROR: Executable file $EXE does not exist!"
      exit 1
   fi
   # Remove the symbolic link to avoid problems when changing the host
   rm $EXE_LN
   # Creates the symbolic link to the executable
   if [ ! -f $EXE_LN ]; then
      ln -s $EXE $EXE_LN
   fi
}

# -------------------------------------
# Checks the number of executions fo the configuration
# Example:
# ../xxx -u Cmdenv -f ../omnetpp.ini -x exp1 -g 
# -------------------------------------
function query_runs() {
   echo "Runs of configuration $CONFIG ..."
   (cd $PATH_PRJ_SRC && $EXE_LN -u Cmdenv -f $PATH_PRJ_SIM/omnetpp.ini -x $CONFIG -g)
   echo ""
}

# -------------------------------------
# Delete previous results
# Example:
# rm ../results/static0/*.vec ../results/static0/*.sca 2>/dev/null
# -------------------------------------
function delete_results() {
   echo "Deleting previous results ..."
   rm $PATH_PRJ_SIM_RESULTS/$CONFIG-*.sca 2>/dev/null
   rm $PATH_PRJ_SIM_RESULTS/$CONFIG-*.vci 2>/dev/null
   rm $PATH_PRJ_SIM_RESULTS/$CONFIG-*.vec 2>/dev/null
   echo ""
}

# -------------------------------------
# Run the simulations depending on the number of cores:
# - Secuentially:
#   xxx -u Cmdenv -f omnetpp.ini -c exp1 -r 0..2
# - In parallel:
#   opp_runall -j2 xxx -u Cmdenv -f omnetpp.ini -c exp1 -r 0..2
# -------------------------------------
function run_simulations() {
   # NEDPATH environment variable to find the .ned files
   export NEDPATH=$PATH_PRJ_VEINS:$PATH_PRJ_SRC:$PATH_PRJ_SIMS
   #echo "NEDPATH = $NEDPATH"

   COMMAND_RUN="$PRJ_NAME -u Cmdenv -f omnetpp.ini -c $CONFIG -r $FIRSTRUN..$LASTRUN"

   # Set the working path to the parent directory and run
   if [ $NUM_CORES -le 1 ]; then
      echo "Sequentially ..."
      $COMMAND_RUN
   else
      echo "In parallel ($NUM_CORES cores) ..."
      # opp_runall -j1 ppp_qos -u Cmdenv -f omnetpp.ini -c exp1 -r 0..4
      opp_runall -j$NUM_CORES $COMMAND_RUN
   fi
}

# -------------------------------------
# For debugging (first run specified).
# NOTE: in the omnetpp.ini file we should write:
#    debug-on-errors = true
# When the gdb opens type 'run':
#    (gdb) run
# Alternative to avoid typying 'run':
#    ggdb -ex=run --args ...
# For quit from gdb type 'quit':
#    (gdb) quit
# -------------------------------------
function debug_simulation() {
   # Configuramos la variable de entorno NEDPATH para que encuentre archivos .ned
   export NEDPATH=$PATH_PRJ_VEINS:$PATH_PRJ_SRC:$PATH_PRJ_SIMS
   echo "VEINS=$PATH_PRJ_VEINS"
   echo "NEDPATH=$NEDPATH"

   (cd .. && $GDB -ex=run --args $PRJ_NAME -u Cmdenv -f omnetpp.ini -c $CONFIG -r $FIRSTRUN)
}

# -------------------------------------

# Arguments
CONFIG=$1
FIRSTRUN=$2
LASTRUN=$3
if [ $# -eq 4 ]; then
   NUM_CORES=$4
else
   NUM_CORES=2
fi

# Show paths
show_paths
# Makes link to the executable file
make_link_exe
# Show runs
query_runs

# Exit if only the configuration name was specified
if [ $# -lt 3 ]; then
   exit 0
fi

if [ $FIRSTRUN -gt $LASTRUN ]; then
   echo "ERROR: first run $FIRSTRUN is greater than last run $LASTRUN!"
   exit 1
fi

confirm
#delete_results

# The internal variable SECONDS is used to measure the real simulation time
SECONDS=0
#debug_simulation
run_simulations
echo "Finished - $SECONDS seconds"

