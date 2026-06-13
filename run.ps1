Write-Host "Compiling Car Dodge Game..."

g++ main.cpp -o CarDodgeGame.exe -lfreeglut -lopengl32 -lglu32

if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful. Starting game..."
    .\CarDodgeGame.exe
} else {
    Write-Host "Compilation failed. Please check the errors above."
}
