@echo off

start nodemon -w build/generate.exe -w pages/ -w assets/ -w code/ -e css,js,html,md -x "build\generate.exe"
pushd generated
start python -m http.server
popd
