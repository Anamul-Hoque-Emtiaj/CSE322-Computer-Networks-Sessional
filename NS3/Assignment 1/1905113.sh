#!/bin/bash

function run_ns3_program() {
    local cc_file=$1
    local exp_name=$2
    local integersArg=$3

    ./ns3 run $cc_file -- --exp_name="$exp_name" --integersArg="$integersArg"
}

function plot_combined_graph() {
    local exp_name=$1
    local x_label=$2
    local y_label=$3
    local data_dir=$4
    # Set output file path to be in the same directory as the data file
    local output_file="${data_dir}/${exp_name}.png"
    if [ -d "$data_dir" ]; then
        plot_command=""
        for data_file in "$data_dir"/*.dat; do
            plot_command+="'${data_file}' using 1:2 with lines title '$(basename "$data_file" .dat)',"
        done

        # Remove the trailing comma from the plot_command
        plot_command=${plot_command%,}
        #echo "$plot_command"

        gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output '$output_file'; \
            set title '$exp_name'; \
            set xlabel '$x_label'; \
            set ylabel '$y_label'; \
            plot $plot_command"
    else
        echo "Error: provided directory not found. Cannot generate the plot."
    fi
}

function run_static(){
    ./ns3 run scratch/1905113_1.cc -- --exp_name=node --integersArg=20,40,60,80,100
    ./ns3 run scratch/1905113_1.cc -- --exp_name=flow --integersArg=10,20,30,40,50
    ./ns3 run scratch/1905113_1.cc -- --exp_name=pps --integersArg=100,200,300,400,500
    ./ns3 run scratch/1905113_1.cc -- --exp_name=ca --integersArg=1,2,3,4,5

    echo "Making Graph..."
    
    plot_graph "Node vs Throughput" "Number of Nodes" "Throughput (kbps)" "scratch/1905113/static/node/throughput.dat"
    plot_graph "Node vs Delivery Ratio" "Number of Nodes" "Delivery Ratio (%)" "scratch/1905113/static/node/delivery_ratio.dat"
    plot_graph "Flow vs Throughput" "Number of Flows" "Throughput (kbps)" "scratch/1905113/static/flow/throughput.dat"
    plot_graph "Flow vs Delivery Ratio" "Number of Flows" "Delivery Ratio (%)" "scratch/1905113/static/flow/delivery_ratio.dat"
    plot_graph "Packet per Second vs Throughput" "Packet per Second" "Throughput (kbps)" "scratch/1905113/static/pps/throughput.dat"
    plot_graph "Packet per Second vs Delivery Ratio" "Packet per Second" "Delivery Ratio (%)" "scratch/1905113/static/pps/delivery_ratio.dat"
    plot_graph "Coverage Area vs Throughput" "Coverage Area" "Throughput (kbps)" "scratch/1905113/static/ca/throughput.dat"
    plot_graph "Coverage Area vs Delivery Ratio" "Coverage Area" "Delivery Ratio (%)" "scratch/1905113/static/ca/delivery_ratio.dat"

    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/node/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/node/delivery_ratio"
    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/flow/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/flow/delivery_ratio"
    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/pps/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/pps/delivery_ratio"
    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/ca/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/ca/delivery_ratio"

}

function run_mobile(){
    ./ns3 run scratch/1905113_2.cc -- --exp_name=node --integersArg=20,40,60,80,100
    ./ns3 run scratch/1905113_2.cc -- --exp_name=flow --integersArg=10,20,30,40,50
    ./ns3 run scratch/1905113_2.cc -- --exp_name=pps --integersArg=100,200,300,400,500
    ./ns3 run scratch/1905113_2.cc -- --exp_name=speed --integersArg=5,10,15,20,25
    echo "Making Graph..."
    plot_graph "Node vs Throughput" "Number of Nodes" "Throughput (kbps)" "scratch/1905113/mobile/node/throughput.dat"
    plot_graph "Node vs Delivery Ratio" "Number of Nodes" "Delivery Ratio (%)" "scratch/1905113/mobile/node/delivery_ratio.dat"
    plot_graph "Flow vs Throughput" "Number of Flows" "Throughput (kbps)" "scratch/1905113/mobile/flow/throughput.dat"
    plot_graph "Flow vs Delivery Ratio" "Number of Flows" "Delivery Ratio (%)" "scratch/1905113/mobile/flow/delivery_ratio.dat"
    plot_graph "Packet per Second vs Throughput" "Packet per Second" "Throughput (kbps)" "scratch/1905113/mobile/pps/throughput.dat"
    plot_graph "Packet per Second vs Delivery Ratio" "Packet per Second" "Delivery Ratio (%)" "scratch/1905113/mobile/pps/delivery_ratio.dat"
    plot_graph "Speed vs Throughput" "Speed (m/s)" "Throughput (kbps)" "scratch/1905113/mobile/speed/throughput.dat"
    plot_graph "Speed vs Delivery Ratio" "Speed (m/s)" "Delivery Ratio (%)" "scratch/1905113/mobile/speed/delivery_ratio.dat"

    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/node/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/node/delivery_ratio"
    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/flow/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/flow/delivery_ratio"
    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/pps/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/pps/delivery_ratio"
    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/speed/throughput"
    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/speed/delivery_ratio"
}

function plot_graph() {
    local exp_name=$1
    local x_label=$2
    local y_label=$3
    local data_file=$4
    # Extract the directory path from the data_file
    data_dir=$(dirname "$data_file")
    # Set output file path to be in the same directory as the data file
    local output_file="${data_dir}/${exp_name}.png"
    if [ -f "$data_file" ]; then
        gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output '$output_file'; \
            set title '$exp_name'; \
            set xlabel '$x_label'; \
            set ylabel '$y_label'; \
            plot '$data_file' using 1:2 with lines title 'Data'"
    else
        echo "Error: $exp_name input file not found. Cannot generate the plot."
    fi
}

function print_menu() {
    echo "Type 1 for testing Wireless high-rate (e.g., 802.11) (static)"
    echo "Type 2 for testing Wireless high-rate (e.g., 802.11) (mobile)"
    echo "Type 3 for running both file with all variable options and generating plot in one command"
    echo "Type 4 for exit"
}

function print_menu_wireless_mobile() {
    echo "Type 0 for fully customize run"
    echo "Type 1 for runing all variable option and generating plot in one command"
    echo "Type 2 for variable node option"
    echo "Type 3 for variable flow option"
    echo "Type 4 for variable packet per second option"
    echo "Type 5 for variable speed option"
    echo "Type 6 for making graph"
    echo "Type 7 for exit"
}

function print_menu_wireless_static() {
    echo "Type 0 for fully customize run"
    echo "Type 1 for runing all variable option and generating plot in one command"
    echo "Type 2 for variable node option"
    echo "Type 3 for variable flow option"
    echo "Type 4 for variable packet per second option"
    echo "Type 5 for variable coverage area option"
    echo "Type 6 for making graph"
    echo "Type 7 for exit"
}

function read_integer() {
    read -p "Enter an integer: " option
    echo "$option"
}

function read_array_of_integers() {
    read -p "Enter an array of integers (comma-separated): " integersArg
    echo "$integersArg"
}

wireless_type=1
main_dir="scratch/1905113"
if [ ! -d "$main_dir" ]; then
    mkdir "$main_dir"
fi

main_dir="scratch/1905113/static"
if [ ! -d "$main_dir" ]; then
    mkdir "$main_dir"
fi

main_dir="scratch/1905113/mobile"
if [ ! -d "$main_dir" ]; then
    mkdir "$main_dir"
fi

while true; do
    print_menu
   
    option=$(read_integer)
    
    case $option in
        1)
            wireless_type=1
            ;;
        2)
            wireless_type=2
            ;;
        3)
            run_static
            run_mobile
            echo "Exiting..."
            exit 0
            ;;
	    4)
	        echo "Exiting..."
            exit 0
            ;;
        *)
            echo "Invalid option. Please try again."
            continue
            ;;
    esac

    while true; do
        if [ $wireless_type -eq 1 ]; then
            print_menu_wireless_static
        else
            print_menu_wireless_mobile
        fi
        
        sub_option=$(read_integer)

        case $sub_option in
            0)
                echo "Enter Number of Node:"
                read node

                echo "Enter Number of Flow:"
                read flow

                echo "Enter Number of PPS:"
                read pps
                if [ $wireless_type -eq 1 ]; then
                    echo "Enter Number of Coverage Area:"
                    read ca
                    ./ns3 run scratch/1905113_1.cc -- --node="$node" --flow="$flow" --pps="$pps" --ca="$ca"
                    plot_combined_graph "Time vs Throughput" "Time (s)" "Throughput (kbps)" "scratch/1905113/static/default/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time (s)" "Delivery Ratio (%)" "scratch/1905113/static/default/delivery_ratio"


                else
                    echo "Enter Number of Speed:"
                    read speed
                    ./ns3 run scratch/1905113_2.cc -- --node="$node" --flow="$flow" --pps="$pps" --speed="$speed"
                    plot_combined_graph "Time vs Throughput" "Time (s)" "Throughput (kbps)" "scratch/1905113/mobile/default/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time (s)" "Delivery Ratio (%)" "scratch/1905113/mobile/default/delivery_ratio"
                fi
                ;;
            1)
                if [ $wireless_type -eq 1 ]; then
                    run_static
                else
                    run_mobile
                fi
                ;;
            2)
                integersArg=$(read_array_of_integers)
		        if [ $wireless_type -eq 1 ]; then
            		run_ns3_program "scratch/1905113_1.cc" "node" "$integersArg"
       		    else
           		    run_ns3_program "scratch/1905113_2.cc" "node" "$integersArg"
        	    fi
                ;;
            3)
                integersArg=$(read_array_of_integers)
		        if [ $wireless_type -eq 1 ]; then
            		run_ns3_program "scratch/1905113_1.cc" "flow" "$integersArg"
                else
                    run_ns3_program "scratch/1905113_2.cc" "flow" "$integersArg"
                fi
                ;;
            4)
                integersArg=$(read_array_of_integers)
                if [ $wireless_type -eq 1 ]; then
            		run_ns3_program "scratch/1905113_1.cc" "pps" "$integersArg"
                else
                    run_ns3_program "scratch/1905113_2.cc" "pps" "$integersArg"
                fi
                ;;
            5)
                integersArg=$(read_array_of_integers)
                if [ $wireless_type -eq 1 ]; then
            		run_ns3_program "scratch/1905113_1.cc" "ca" "$integersArg"
                else
                    run_ns3_program "scratch/1905113_2.cc" "speed" "$integersArg"
                fi
                ;;
            6)
                echo "Making graphs..."
                if [ $wireless_type -eq 1 ]; then
                    plot_graph "Node vs Throughput" "Number of Nodes" "Throughput (kbps)" "scratch/1905113/static/node/throughput.dat"
                    plot_graph "Node vs Delivery Ratio" "Number of Nodes" "Delivery Ratio (%)" "scratch/1905113/static/node/delivery_ratio.dat"
                    plot_graph "Flow vs Throughput" "Number of Flows" "Throughput (kbps)" "scratch/1905113/static/flow/throughput.dat"
                    plot_graph "Flow vs Delivery Ratio" "Number of Flows" "Delivery Ratio (%)" "scratch/1905113/static/flow/delivery_ratio.dat"
                    plot_graph "Packet per Second vs Throughput" "Packet per Second" "Throughput (kbps)" "scratch/1905113/static/pps/throughput.dat"
                    plot_graph "Packet per Second vs Delivery Ratio" "Packet per Second" "Delivery Ratio (%)" "scratch/1905113/static/pps/delivery_ratio.dat"
                    plot_graph "Coverage Area vs Throughput" "Coverage Area" "Throughput (kbps)" "scratch/1905113/static/ca/throughput.dat"
                    plot_graph "Coverage Area vs Delivery Ratio" "Coverage Area" "Delivery Ratio (%)" "scratch/1905113/static/ca/delivery_ratio.dat"

                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/node/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/node/delivery_ratio"
                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/flow/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/flow/delivery_ratio"
                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/pps/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/pps/delivery_ratio"
                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/static/ca/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/static/ca/delivery_ratio"
                else
                    plot_graph "Node vs Throughput" "Number of Nodes" "Throughput (kbps)" "scratch/1905113/mobile/node/throughput.dat"
                    plot_graph "Node vs Delivery Ratio" "Number of Nodes" "Delivery Ratio (%)" "scratch/1905113/mobile/node/delivery_ratio.dat"
                    plot_graph "Flow vs Throughput" "Number of Flows" "Throughput (kbps)" "scratch/1905113/mobile/flow/throughput.dat"
                    plot_graph "Flow vs Delivery Ratio" "Number of Flows" "Delivery Ratio (%)" "scratch/1905113/mobile/flow/delivery_ratio.dat"
                    plot_graph "Packet per Second vs Throughput" "Packet per Second" "Throughput (kbps)" "scratch/1905113/mobile/pps/throughput.dat"
                    plot_graph "Packet per Second vs Delivery Ratio" "Packet per Second" "Delivery Ratio (%)" "scratch/1905113/mobile/pps/delivery_ratio.dat"
                    plot_graph "Speed vs Throughput" "Speed (m/s)" "Throughput (kbps)" "scratch/1905113/mobile/speed/throughput.dat"
                    plot_graph "Speed vs Delivery Ratio" "Speed (m/s)" "Delivery Ratio (%)" "scratch/1905113/mobile/speed/delivery_ratio.dat"

                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/node/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/node/delivery_ratio"
                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/flow/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/flow/delivery_ratio"
                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/pps/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/pps/delivery_ratio"
                    plot_combined_graph "Time vs Throughput" "Time(s)" "Throughput (kbps)" "scratch/1905113/mobile/speed/throughput"
                    plot_combined_graph "Time vs Delivery Ratio" "Time(s)" "Delivery Ratio (%)" "scratch/1905113/mobile/speed/delivery_ratio"
                fi
                ;;
            7)
                exit 0
                ;;
            *)
                echo "Invalid option. Please try again."
                ;;
        esac
    done
done
