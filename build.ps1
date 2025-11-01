param(
    [string]$Config = "Release",
    [string]$VcpkgToolchain
)

if (-not $VcpkgToolchain) {
    $VcpkgToolchain = if ($env:VCPKG_TOOLCHAIN_FILE) { $env:VCPKG_TOOLCHAIN_FILE } else { "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" }
}

Write-Host "Building $Config configuration..." -ForegroundColor Cyan

cmake -B "build/$Config" -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=$Config -DCMAKE_TOOLCHAIN_FILE="$VcpkgToolchain"
cmake --build "build/$Config" --config $Config

$OutputPath = "$PWD\build\$Config\AqMD3_console\$Config\"
Write-Host "Application build output folder $OutputPath" -ForegroundColor Cyan