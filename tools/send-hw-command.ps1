param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Command,

    [Parameter(Position = 1)]
    [string]$Port = "COM3",

    [int]$Baud = 115200
)

$serial = New-Object System.IO.Ports.SerialPort $Port, $Baud, "None", 8, "One"
$serial.ReadTimeout = 2000
$serial.WriteTimeout = 2000
$serial.DtrEnable = $true
$serial.RtsEnable = $true

try {
    $serial.Open()
    Start-Sleep -Milliseconds 250
    $serial.DiscardInBuffer()
    $serial.WriteLine($Command)
    Start-Sleep -Milliseconds 900
    $serial.ReadExisting()
}
finally {
    if ($serial.IsOpen) {
        $serial.Close()
    }
}
