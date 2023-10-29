#!/bin/bash
rm -rf scratch/1905113
mkdir scratch/1905113
mkdir scratch/1905113/bdr
mkdir scratch/1905113/plr
mkdir scratch/1905113/cwnd
mkdir scratch/1905113/pcap
mkdir scratch/1905113/bdr/newreno-vs-westwoodplus
mkdir scratch/1905113/bdr/newreno-vs-highspeed
mkdir scratch/1905113/bdr/newreno-vs-adaptivereno
mkdir scratch/1905113/plr/newreno-vs-westwoodplus
mkdir scratch/1905113/plr/newreno-vs-highspeed
mkdir scratch/1905113/plr/newreno-vs-adaptivereno
mkdir scratch/1905113/cwnd/newreno-vs-westwoodplus
mkdir scratch/1905113/cwnd/newreno-vs-highspeed
mkdir scratch/1905113/cwnd/newreno-vs-adaptivereno


./ns3 run 'scratch/1905113.cc' -- --exp_name="bdr" --numbersArg="0.001,0.01,0.1,1,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200,210,220,230,240,250,260,270,280,290,300"
./ns3 run 'scratch/1905113.cc' -- --exp_name="plr" --numbersArg="0.01,0.005,0.001,0.0005,0.0001,0.00005,0.00001,0.000005,0.000001"
./ns3 run 'scratch/1905113.cc'
./ns3 run 'fifth.cc'

echo "Making Graph"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/bdr/newreno-vs-westwoodplus/throughput.png'; \
            set title 'Bottleneck-Data-Rate-vs-Throughput'; \
            set ylabel 'Throughput(kbps)'; \
            set xlabel 'Bottleneck-Data-Rate(Mbps)'; \
            plot 'scratch/1905113/bdr/newreno-vs-westwoodplus/1st_type.dat' using 1:2 with lines title 'new-reno','scratch/1905113/bdr/newreno-vs-westwoodplus/2nd_type.dat' using 1:2 with lines title 'WestWoodPlus'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/bdr/newreno-vs-westwoodplus/fairness-index.png'; \
            set title 'Bottleneck-Data-Rate-vs-Fairness-Index'; \
            set xlabel 'Bottleneck-Data-Rate(Kbps)'; \
            set ylabel 'Fairness-Index'; \
            plot 'scratch/1905113/bdr/newreno-vs-westwoodplus/fairness_index.dat' using 1:2 with lines title 'Fairness-Index'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/bdr/newreno-vs-highspeed/throughput.png'; \
            set title 'Bottleneck-Data-Rate-vs-Throughput'; \
            set ylabel 'Throughput(kbps)'; \
            set xlabel 'Bottleneck-Data-Rate(Mbps)'; \
            plot 'scratch/1905113/bdr/newreno-vs-highspeed/1st_type.dat' using 1:2 with lines title 'new-reno','scratch/1905113/bdr/newreno-vs-highspeed/2nd_type.dat' using 1:2 with lines title 'HighSpeed'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/bdr/newreno-vs-highspeed/fairness-index.png'; \
            set title 'Bottleneck-Data-Rate-vs-Fairness-Index'; \
            set xlabel 'Bottleneck-Data-Rate(Kbps)'; \
            set ylabel 'Fairness-Index'; \
            plot 'scratch/1905113/bdr/newreno-vs-highspeed/fairness_index.dat' using 1:2 with lines title 'Fairness-Index'"


gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/bdr/newreno-vs-adaptivereno/throughput.png'; \
            set title 'Bottleneck-Data-Rate-vs-Throughput'; \
            set ylabel 'Throughput(kbps)'; \
            set xlabel 'Bottleneck-Data-Rate(Mbps)'; \
            plot 'scratch/1905113/bdr/newreno-vs-adaptivereno/1st_type.dat' using 1:2 with lines title 'new-reno','scratch/1905113/bdr/newreno-vs-adaptivereno/2nd_type.dat' using 1:2 with lines title 'Adaptive-Reno'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/bdr/newreno-vs-adaptivereno/fairness-index.png'; \
            set title 'Bottleneck-Data-Rate-vs-Fairness-Index'; \
            set xlabel 'Bottleneck-Data-Rate(Kbps)'; \
            set ylabel 'Fairness-Index'; \
            plot 'scratch/1905113/bdr/newreno-vs-adaptivereno/fairness_index.dat' using 1:2 with lines title 'Fairness-Index'"





gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/plr/newreno-vs-westwoodplus/throughput.png'; \
            set title 'Packet-Loss-Rate-vs-Throughput'; \
            set ylabel 'Throughput(kbps)'; \
            set xlabel 'Packet-Loss-Rate'; \
            plot 'scratch/1905113/plr/newreno-vs-westwoodplus/1st_type.dat' using 1:2 with lines title 'new-reno','scratch/1905113/plr/newreno-vs-westwoodplus/2nd_type.dat' using 1:2 with lines title 'WestWoodPlus'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/plr/newreno-vs-westwoodplus/fairness-index.png'; \
            set title 'Packet-Loss-Rate-vs-Fairness-Index'; \
            set xlabel 'Packet-Loss-Rate'; \
            set ylabel 'Fairness-Index'; \
            plot 'scratch/1905113/plr/newreno-vs-westwoodplus/fairness_index.dat' using 1:2 with lines title 'Fairness-Index'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/plr/newreno-vs-highspeed/throughput.png'; \
            set title 'Packet-Loss-Rate-vs-Throughput'; \
            set ylabel 'Throughput(kbps)'; \
            set xlabel 'Packet-Loss-Rate'; \
            plot 'scratch/1905113/plr/newreno-vs-highspeed/1st_type.dat' using 1:2 with lines title 'new-reno','scratch/1905113/plr/newreno-vs-highspeed/2nd_type.dat' using 1:2 with lines title 'HighSpeed'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/plr/newreno-vs-highspeed/fairness-index.png'; \
            set title 'Packet-Loss-Rate-vs-Fairness-Index'; \
            set xlabel 'Packet-Loss-Rate'; \
            set ylabel 'Fairness-Index'; \
            plot 'scratch/1905113/plr/newreno-vs-highspeed/fairness_index.dat' using 1:2 with lines title 'Fairness-Index'"


gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/plr/newreno-vs-adaptivereno/throughput.png'; \
            set title 'Packet-Loss-Rate-vs-Throughput'; \
            set ylabel 'Throughput(kbps)'; \
            set xlabel 'Packet-Loss-Rate'; \
            plot 'scratch/1905113/plr/newreno-vs-adaptivereno/1st_type.dat' using 1:2 with lines title 'new-reno','scratch/1905113/plr/newreno-vs-adaptivereno/2nd_type.dat' using 1:2 with lines title 'Adaptive-Reno'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/plr/newreno-vs-adaptivereno/fairness-index.png'; \
            set title 'Packet-Loss-Rate-vs-Fairness-Index'; \
            set xlabel 'Packet-Loss-Rate'; \
            set ylabel 'Fairness-Index'; \
            plot 'scratch/1905113/plr/newreno-vs-adaptivereno/fairness_index.dat' using 1:2 with lines title 'Fairness-Index'"





gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/cwnd/newreno-vs-adaptivereno/congestionWindow.png'; \
            set title 'Time-vs-CongestionWindow'; \
            set xlabel 'Time(s)'; \
            set ylabel 'CongestionWindow'; \
            plot 'scratch/1905113/cwnd/newreno-vs-adaptivereno/1st_type_cwnd.dat' using 1:2 with lines title 'new-reno','scratch/1905113/cwnd/newreno-vs-adaptivereno/2nd_type_cwnd.dat' using 1:2 with lines title 'Adaptive-Reno'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/cwnd/newreno-vs-highspeed/congestionWindow.png'; \
            set title 'Time-vs-CongestionWindow'; \
            set xlabel 'Time(s)'; \
            set ylabel 'CongestionWindow'; \
            plot 'scratch/1905113/cwnd/newreno-vs-highspeed/1st_type_cwnd.dat' using 1:2 with lines title 'new-reno','scratch/1905113/cwnd/newreno-vs-highspeed/2nd_type_cwnd.dat' using 1:2 with lines title 'Highspeed'"

gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/cwnd/newreno-vs-westwoodplus/congestionWindow.png'; \
            set title 'Time-vs-CongestionWindow'; \
            set xlabel 'Time(s)'; \
            set ylabel 'CongestionWindow'; \
            plot 'scratch/1905113/cwnd/newreno-vs-westwoodplus/1st_type_cwnd.dat' using 1:2 with lines title 'new-reno','scratch/1905113/cwnd/newreno-vs-westwoodplus/2nd_type_cwnd.dat' using 1:2 with lines title 'westwoodplus'"


gnuplot -e "set terminal pngcairo enhanced font 'arial,10' fontscale 1.0 size 800, 600; \
            set output 'scratch/1905113/cwnd/CongestionWindow.png'; \
            set title 'Time-vs-CongestionWindow'; \
            set xlabel 'Time(s)'; \
            set ylabel 'CongestionWindow'; \
            plot 'scratch/1905113/cwnd/cwnd.dat' using 1:2 with lines title 'Adaptive-Reno'"