New-Item -Path $PSScriptRoot/compiled_shaders -ItemType Directory -Force

$vuklan_path = $env:VULKAN_SDK

Get-ChildItem -Path $PSScriptRoot/shaders -File -Filter "*.vert" | ForEach-Object {
    $output = "$PSScriptRoot/compiled_shaders/$($_.BaseName)__vert.spv"
    &$vuklan_path\bin\glslc $_.FullName -o $output
}

Get-ChildItem -Path $PSScriptRoot/shaders -File -Filter "*.frag" | ForEach-Object {
    $output = "$PSScriptRoot/compiled_shaders/$($_.BaseName)__frag.spv"
    &$vuklan_path\bin\glslc $_.FullName -o $output
}