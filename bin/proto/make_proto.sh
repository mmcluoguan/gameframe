for %%i in (*.proto) do (  
    protoc --cpp_out=../../frmpub/protocc %%i  
    protoc -o ../lua/pb/%%~ni.pb %%i 
)
pause

