#!/usr/bin/env bash

conf_choices()
{
    configlist=`find configs -name config.h`
    for config in ${configlist}; do
      config=`dirname ${config} | sed -e "s,configs/,,g"`
      printf '\t%s\n' ${config}
    done
    printf '\n'
}

conf_usage()
{
    printf '\nUsage: %s <board>[/<rev>]\n' $0
    printf 'Current choices for <board>[/<rev>] are:\n'
    conf_choices
}

if [[ (! $# -eq 1) || (${1:0:2} == "-h") || (${1:0:3} == "--h") ]]; then
    conf_usage
    exit 1
fi

config_dir=configs/$1
if [[ ! -d ${config_dir} ]]; then
    printf '\nError: %s does not exist\n\n' ${config_dir}
    printf 'Your choices are:\n'
    conf_choices
    exit 2
fi

# Verify Make.defs & config.h are in the ${config_dir}
if [[ ! -r ${config_dir}/config.h ]]; then
    printf '\nError: %s not found\n\n' ${config_dir}/config.h
    exit 2
fi

if [[ ! -r ${config_dir}/Make.defs ]]; then
    printf '\nError: %s not found\n\n' ${config_dir}/Make.defs
    exit 2
fi

# Soft-link Make.defs & config.h from the ${config_dir} here
rm -f config.h
ln -s ${config_dir}/config.h
if [[ ! $? -eq 0 ]]; then
    printf '\nPerhaps a \"make distclean\" is in order?\n\n'
    exit 3
fi

rm -f Make.defs
ln -s ${config_dir}/Make.defs
if [[ ! $? -eq 0 ]]; then
    printf '\nPerhaps a \"make distclean\" is in order?\n\n'
    exit 3
fi

exit 0
