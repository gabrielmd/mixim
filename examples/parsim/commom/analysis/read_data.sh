#!/bin/bash

foldername="../../results"
output="data.m"

while getopts "f:d:o:" VALUE
do
  	case $VALUE in
	  f)	
	  		if [[ ! -z ${OPTARG} ]]; then
	  			filename=${OPTARG}
	  		fi
	  		;;
	  d)	
	  		if [[ ! -z ${OPTARG} ]]; then
	  			foldername=${OPTARG}
	  		fi
	  		;;
	  o)	  		
			if [[ ! -z ${OPTARG} ]]; then
	  			output=${OPTARG}
	  		fi
	  		;;
	  ?)	
			;;
	esac
done

rm -f $output

for VAR in  "nbDataPacketsForwarded" "nbDataPacketsReceived" "nbDataPacketsSent" ".*Mean power consumption" ".*device total \(mWs\)"
do

	for f in $foldername/$filename-*.sca
	do

		regex="$filename-([0-9]+).sca"
		if [[ $f =~ $regex ]]
   		then
			run_nr=$(echo ${BASH_REMATCH[1]} + 1 | bc)
		fi

		value=0
		regex="^scalar\ .*\\s$VAR.*\\s([0-9]+(\\.[0-9]+)?)$"
		while read LINE
		do
			if [[ $LINE =~ $regex ]]
	   		then
				value=$(echo "${BASH_REMATCH[1]} + $value" | bc)
			fi
		done < $f
		
		clean_name=${VAR//\./}
		clean_name=${clean_name//\*/}
		clean_name=${clean_name//\ /}
		clean_name=${clean_name//\\/}
		clean_name=${clean_name//\(/}
		clean_name=${clean_name//\)/}
		echo "$clean_name($run_nr) = $value;" >> $output
	done
done


