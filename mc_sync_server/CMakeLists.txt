# CMakeList.txt: CMake-Projekt für "mc_sync_server". Schließen Sie die Quelle ein, und definieren Sie
# projektspezifische Logik hier.
#
cmake_minimum_required (VERSION 3.8)

# Fügen Sie der ausführbaren Datei dieses Projekts eine Quelle hinzu.
add_executable (mc_sync_server "mc_sync_server.cpp" "mc_sync_server.h" "serverConnection.h" "serverConnection.cpp" "myexceptions.h")

# TODO: Fügen Sie bei Bedarf Tests hinzu, und installieren Sie Ziele.
