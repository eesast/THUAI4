cd CAPI/CAPI
rm -f /usr/local/mnt/player/PLAYER*
make SOURCE_DIR=/usr/local/mnt/cpp BIN_DIR=/usr/local/mnt/player/ &>  /usr/local/mnt/player/out.log
cd /usr/local/mnt/player
if [ -f ./PLAYER1 ] && [ -f ./PLAYER2 ] && [ -f ./PLAYER3 ] && [ -f ./PLAYER4 ] 
then 
curl -X PUT -H 'Content-Type: application/json' -H "Authorization: Bearer ${COMPILER_TOKEN}" -d '{"compile_status":"compiled"}' https://api.eesast.com/code/compileInfo/
else 
curl -X PUT -H 'Content-Type: application/json' -H "Authorization: Bearer ${COMPILER_TOKEN}" -d '{"compile_status":"failed"}' https://api.eesast.com/code/compileInfo/
fi
exit 0
