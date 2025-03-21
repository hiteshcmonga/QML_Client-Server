#!/bin/bash
# Cross-platform build script for QML Client-Server Suite

# Add Docker build option
if [ "$1" == "docker" ]; then
    docker-compose build
    exit 0
fi

# Create build directory
mkdir -p build
cd build

# Standard build commands
cmake ../app
cmake --build .

# Platform-specific post-build steps
case "$(uname -s)" in
    Linux*)
        # Linux-specific: Set QML2_IMPORT_PATH
        export QML2_IMPORT_PATH="${QML2_IMPORT_PATH}:$(qtpaths --import-path)"
        echo "Linux build complete. Ensure QtQuick.Controls is installed system-wide."
        ;;
    MINGW*|MSYS*|CYGWIN*)
        # Windows-specific deployment
        if command -v windeployqt >/dev/null 2>&1; then
            windeployqt --qmldir ../server/qml_files Application.exe
        else
            QT_PATHS=(
                "/c/Qt/6.8.2/mingw_64/bin"
                "/c/Qt/6.7.0/mingw_64/bin"
                "$HOME/Qt/6.8.2/mingw_64/bin"
            )
            
            for path in "${QT_PATHS[@]}"; do
                if [ -f "${path}/windeployqt.exe" ]; then
                    "${path}/windeployqt.exe" --qmldir ../server/qml_files Application.exe
                    break
                fi
            done
        fi
        ;;
esac

cd ..

# Run instructions
echo "Build complete."
echo "To run the application:"
case "$(uname -s)" in
    Linux*)
        echo "  ./build/Application"
        echo "Note: Ensure Qt 6 Quick Components are installed:"
        echo "  sudo apt-get install qml6-module-qtquick-controls"
        ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "  ./build/Application.exe"
        ;;
    *)
        echo "  ./build/Application"
        ;;
esac
echo "The C++ application will automatically launch the Python server."