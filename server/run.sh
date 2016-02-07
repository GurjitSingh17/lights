#!/bin/bash

function act (){
        source "$1/Scripts/activate"
}

act venv
python serial_server.py &
python web_server.py > /dev/null &
deactivate