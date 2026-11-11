set -e

echo "Building AEGIS..."

cmake -S . -B build

cmake --build build

echo "Running tests..."

./build/aegis_tests

echo "Starting up AEGIS!"

./build/aegis