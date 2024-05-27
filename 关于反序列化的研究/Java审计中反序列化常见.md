# 代码审计

在Java代码审计中，发现反序列化漏洞的常见情况和代码模式包括以下几种。当遇到这些情况时，需要特别注意并进一步检查：

### 1. 直接反序列化不受信任的数据

直接从用户输入或不受信任的数据源获取数据并进行反序列化，是反序列化漏洞的常见根源。

#### 示例代码：

```java
ObjectInputStream ois = new ObjectInputStream(inputStream);
Object obj = ois.readObject();  // 直接反序列化不受信任的数据
```

### 2. 从不受信任的外部源反序列化数据

从文件、数据库、网络等不受信任的外部源读取数据并反序列化。

#### 示例代码：

```java
FileInputStream fileIn = new FileInputStream("data.ser");
ObjectInputStream in = new ObjectInputStream(fileIn);
Object obj = in.readObject();  // 从文件读取数据并反序列化
```

### 3. 反序列化数据后调用危险方法

反序列化的数据被直接用于调用敏感方法，如执行系统命令、文件操作等。

#### 示例代码：

```java
ObjectInputStream ois = new ObjectInputStream(inputStream);
CommandObject cmdObj = (CommandObject) ois.readObject();
cmdObj.execute();  // 反序列化后立即执行敏感方法
```

### 4. 反序列化含有危险类的对象

反序列化过程中使用了可能被攻击者利用的危险类库或类。

#### 示例代码：

```java
import org.apache.commons.collections.map.TransformedMap;

// 反序列化恶意对象，可能利用Apache Commons Collections库中的漏洞
ObjectInputStream ois = new ObjectInputStream(inputStream);
Object obj = ois.readObject();
```

### 5. 使用可反序列化的类且包含危险方法

包含如`readObject`、`readResolve`、`writeReplace`等方法的类，这些方法可能被利用执行恶意代码。

#### 示例代码：

```java
public class VulnerableClass implements Serializable {
    private void readObject(ObjectInputStream ois) throws IOException, ClassNotFoundException {
        ois.defaultReadObject();
        // 可能包含恶意代码
    }
}
```