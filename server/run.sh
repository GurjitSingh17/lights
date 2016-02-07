#!/bin/bash

function act (){
        source "$1/Scripts/activate"
}

act venv
python server.py &
python main.py > /dev/null &
deactivate

