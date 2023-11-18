@echo off
if not exist ..\..\build\asset_builder mkdir ..\..\build\asset_builder
pushd  ..\..\build\asset_builder
cl ..\..\src\asset_builder\asset_builder.c
popd..