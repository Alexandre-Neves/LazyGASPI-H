rm -f lazygaspi_h_*.out
rm -f ~/lazygaspi_h_*.out
ssh alexandrepc "rm -f ~/lazygaspi_h_*.out"

args="-k 4 -n 2 -r 6"
if [[ $# > 0 ]]; then
    args=$@
fi
echo "../GPI/bin/gaspi_run -m machinefile ${PWD}/bin/test.o $args"
../GPI/bin/gaspi_run -m machinefile ${PWD}/bin/test.o $args
    

