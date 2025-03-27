BUILD_DIR="bin/"  # 실행 파일 저장 경로

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

echo "Starting build process..."

go build -o "$BUILD_DIR/RestAPI" cmd/restapi/main.go

echo "Build process finished."
echo "Executable file is located at: $BUILD_DIR"