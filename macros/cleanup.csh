#!/bin/csh

# cleanup the scheduler garbage and log files
# should remove everything the scheduler produces
# don't use while jobs are being submitted/running

rm -rv *session.xml 
rm -rv *report 
rm -rv *condor 
rm -rv *condor.log 
rm -rv sched*list 
rm -rv *dataset 
rm -rv ZippedRefmult* 
rm -rv sched*csh 
rm -rv scheduler/csh/* 
rm -rv scheduler/list/* 
rm -rv scheduler/report/* 
rm -rv ZIPy14* 
rm -rv *package
rm -rv out/logs/* 
rm -rv out/tmplogs/*
rm -rv libs/*.so 
rm -rv *.tmp 
rm -rv *.zip

rm -rv /gpfs01/star/pwg/imooney/P18ih/picos/pAu_200_production_2015/log/tmplogs/*
rm -rv /gpfs01/star/pwg/imooney/P18ih/picos/pAu_200_production_2015/out/err/trigger_set_0/*
rm -rv /gpfs01/star/pwg/imooney/P18ih/picos/pAu_200_production_2015/out/log/trigger_set_0/*

