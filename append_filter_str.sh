#!/bin/bash

usage()
{
    echo "Usage: $1 -i <elf文件路径> -s <符号名> -o <追加输出的文件路径>"
    exit 0
}

ELF_FILE=""
SYMBOL=""
APPEND_FILE=""

while getopts "i:s:o:h" opt; do
    case $opt in
        i)
            ELF_FILE="$OPTARG"
            ;;
        s)
            SYMBOL="$OPTARG"
            ;;
        o)
            APPEND_FILE="$OPTARG"
            ;;
        \?)
            echo "无效选项: -$OPTARG" >&2
            ;;
        h)
            usage $0
            exit 0
    esac
done

[ -z "${ELF_FILE}" ] && usage $0
[ -z "${SYMBOL}" ] && usage $0
[ -z "${APPEND_FILE}" ] && usage $0

echo "ELF_FILE: ${ELF_FILE}"
echo "SYMBOL: ${SYMBOL}"
echo "APPEND_FILE: ${APPEND_FILE}"

nm  ${ELF_FILE} | \
    awk -v symbol="$SYMBOL"  \
        -v unmatch_symbol="__wrap|__cyg_profile|ctrace_" \
        '$3 !~ unmatch_symbol && $3 ~ symbol {print $1, "-", $3}' >> ${APPEND_FILE}
