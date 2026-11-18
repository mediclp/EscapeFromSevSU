# После правок в src, tests или assets/maps собирает проект и запускает тесты.
# Код выхода 2 передаёт ошибку обратно Claude, чтобы он сразу её исправил.

$payload = [Console]::In.ReadToEnd() | ConvertFrom-Json
$path = $payload.tool_input.file_path
if (-not $path) { exit 0 }

$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$full = [IO.Path]::GetFullPath($path)
$watched = 'src', 'tests', 'assets\maps' | ForEach-Object { Join-Path $root $_ }
$isWatched = $watched | Where-Object { $full.StartsWith("$_\", [StringComparison]::OrdinalIgnoreCase) }
if (-not $isWatched) { exit 0 }

$buildDir = Join-Path $root 'cmake-build-debug'
$cache = Join-Path $buildDir 'CMakeCache.txt'
if (-not (Test-Path $cache)) { exit 0 }  # проект ещё не открывали в CLion

# Пути к инструментам берём из кэша CMake, поэтому скрипт работает у всей команды
function Get-CacheValue($name) {
    $line = Select-String -LiteralPath $cache -Pattern "^${name}:[A-Z]+=(.*)$" | Select-Object -First 1
    if ($line) { $line.Matches[0].Groups[1].Value }
}
$cmake = Get-CacheValue 'CMAKE_COMMAND'
foreach ($tool in 'CMAKE_CXX_COMPILER', 'CMAKE_MAKE_PROGRAM') {
    $env:Path = (Split-Path (Get-CacheValue $tool)) + ';' + $env:Path
}

[Console]::OutputEncoding = [Text.Encoding]::UTF8

# Запущенный .exe нельзя перезаписать, тогда собираем только тесты
$targets = @('game_tests')
if (-not (Get-Process EscapeFromSevSU -ErrorAction SilentlyContinue)) { $targets += 'EscapeFromSevSU' }

$build = & $cmake --build $buildDir --target @targets 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) {
    [Console]::Error.WriteLine("Сборка не прошла:`n$(($build -split "`n" | Select-Object -Last 40) -join "`n")")
    exit 2
}

$tests = & (Join-Path $buildDir 'game_tests.exe') 2>&1 | Out-String
if ($LASTEXITCODE -ne 0) {
    $failed = ($tests -split "`n" | Where-Object { $_ -match 'FAIL|Ошибок' }) -join "`n"
    [Console]::Error.WriteLine("Тесты не прошли:`n$failed")
    exit 2
}
exit 0
