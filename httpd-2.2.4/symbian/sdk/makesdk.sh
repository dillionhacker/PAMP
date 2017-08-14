# Copyright 2008 Nokia Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

function usage
{
    echo "usage: makesdk file"
}


function rmkdir
{
    local parent=${1%/*}
    local child=${1##*/}

    if [ "${parent}" == "${child}" ]
    then
        if [ ! -d ${parent} ]
        then
            mkdir ${parent}
        fi
    else
        if [ ! -d ${parent} ]
        then
            rmkdir ${parent}
        fi

        mkdir ${parent}/${child}
    fi

    if [ $? -ne 0 ]
    then
        exit 1
    fi
}


function create_zip
{
    local sdk=$1

    zip -r ${sdk}/epoc32.zip epoc32

    if [ $? -ne 0 ]
    then
        echo "error: Failed to create epoc32-zip."
        exit 1
    fi

    echo "Unzip epoc32.zip in the same directory where your epoc32 directory resides" > ${sdk}/README.TXT

    zip -r ${sdk}.zip ${sdk}

    if [ $? -ne 0 ]
    then
        echo "error: Failed to create ${sdk}-zip."
        exit 1
    fi
}


function create_entry
{
    local from=$1
    local to=$2

    if [ ! -d ${to} ]
    then
        rmkdir ${to}
    fi

    eval cp ${from} ${to}

    if [ $? -ne 0 ]
    then
        exit 1
    fi

    local file=${to}/${from##*/}
     
    echo ${file}
    local uninstall=$(cygpath -w ${file})
    echo "del ${uninstall}" >> ${sdk}/uninstall.bat

    if [ $? -ne 0 ]
    then
        exit 1
    fi
}


function create_sdk
{
    local in=$1

    local sdk=${1%\.*}_sdk

    mkdir ${sdk}

    if [ $? -ne 0 ]
    then
        exit 1
    fi

    while read from to
    do
       # Ignore comments and empty lines.
       if [ "${from%#*}" != "" -a "${from}" != "" ]
       then
           create_entry ${from} ${to}
       fi
    done < $in

    create_zip ${sdk}
}


function main
{
    if [ $# -eq 0 ]
    then
        usage
        exit 1
    fi

    if [ "${EPOCROOT}" == "" ]
    then
        echo "error: EPOCROOT is not defined."
        exit 1
    fi

    EPOC32=`cygpath ${EPOCROOT}/epoc32`

    if [ ! -d ${EPOC32} ]
    then
        echo "error: ${EPOC32} does not exist or is not a directory."
        exit 1
    fi

    if [ $# -ne 1 ]
    then
        usage
        exit 1
    fi

    if [ ! -f $1 ]
    then
        echo "error: $1 is not a file."
        exit 1
    fi

    create_sdk $1
}


main $*
