#!/usr/bin/env gnuplot

set terminal postscript solid eps monochrome 'Helvetica' 28
set style data linespoints
set ylabel 'Throughput (Mops/s)' font 'Helvetica,35'
set xlabel 'Threads' font 'Helvetica,35'
set pointsize 2
set key width -1
set key horizontal
set key left
set ytic 150
set yrange [0:450]
set xrange[0.8:64.2]
set xtics 8

#set grid y

# glyphs (the 'pt' parameter)
# 1 is plus
# 2 is x
# 3 is star
# 4 is empty square
# 5 is filled square
# 6 is empty circle
# 7 is filled circle
# 8 is empty up triangle
# 9 is filled up triangle	
#"./csv/tbb.u10.i1000000.csv"		u 1:($2/1000) w lp lc rgb "dark-khaki" lw 5 pt 9 ps  2 t 'TBB',\
#"./csv/rcu.u10.i1000000.csv"		u 1:($2/1000) w lp lc rgb "dark-turquoise" lw 5 pt 7 ps 2 t 'URCU', \
#"./csv/clht_lb.u10.i1000000.csv"		u 1:($2/1000) w lp lc rgb "web-blue" lw 5 pt 1 ps  2 t 'CLHT-lb',\
#"./csv/clht_lf.u10.i1000000.csv"		u 1:($2/1000) w lp lc rgb "web-green" lw 5 pt 4 ps 2 t 'CLHT-lf',\
# 10 is empty down triangle
# 11 is full down triangle
# 12 is empty diamond
# 13 is full diamond
# 14 is empty pentagon
# 15 is full pentagon rgb "light-magenta" 
set output './eps/HopU10.eps'
plot  \
	"./csv/hop_htm.u10.i1000000.csv"			u 1:($2/1000) w lp lc rgb "light-red" lw 5 pt 8 ps 2 t 'Hop_htm', \
	"./csv/hop_fine_grained.u10.i1000000.csv"		u 1:($2/1000) w lp lc rgb "orange" lw 5 pt 6 ps 2 t 'Hop_fine_grained'	

set output './eps/HopU0.eps'
plot  \
	"./csv/hop_htm.u0.i1000000.csv"			u 1:($2/1000) w lp lc rgb "light-red" lw 5 pt 8 ps 2 t 'Hop_htm', \
	"./csv/hop_fine_grained.u0.i1000000.csv"		u 1:($2/1000) w lp lc rgb "orange" lw 5 pt 6 ps 2 t 'Hop_fine_grained'	


set output './eps/HopU80.eps'
plot  \
	"./csv/hop_htm.u80.i1000000.csv"			u 1:($2/1000) w lp lc rgb "light-red" lw 5 pt 8 ps 2 t 'Hop_htm', \
	"./csv/hop_fine_grained.u80.i1000000.csv"		u 1:($2/1000) w lp lc rgb "orange" lw 5 pt 6 ps 2 t 'Hop_fine_grained'	


