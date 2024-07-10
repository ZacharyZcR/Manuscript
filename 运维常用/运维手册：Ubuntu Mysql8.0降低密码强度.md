在 MySQL 中降低密码强度要求涉及修改密码验证插件的设置。具体步骤如下：

1. **登录 MySQL 数据库**：
   使用具有管理员权限的账户登录到 MySQL 数据库。

   ```sh
   mysql -u root -p
   ```

2. **查看当前的密码验证插件**：
   执行以下命令查看当前使用的密码验证插件。

   ```sql
   SHOW VARIABLES LIKE 'validate_password%';
   ```

3. **修改密码验证插件的设置**：
   你可以通过修改系统变量来降低密码强度要求，例如降低密码长度、复杂度等。

   ```sql
   -- 禁用密码验证插件（如果需要）
   UNINSTALL PLUGIN validate_password;
   
   -- 或者修改密码验证策略
   SET GLOBAL validate_password.policy=LOW;
   
   -- 设置最小密码长度
   SET GLOBAL validate_password.length=6;
   
   -- 设置密码至少包含的字符类型数量（0表示任何字符类型都可以）
   SET GLOBAL validate_password.mixed_case_count=0;
   SET GLOBAL validate_password.number_count=0;
   SET GLOBAL validate_password.special_char_count=0;
   ```

4. **重启 MySQL 服务**（如果需要）：
   在某些情况下，可能需要重启 MySQL 服务使配置生效。

   ```sh
   sudo service mysql restart
   ```

5. **验证修改是否生效**：
   再次查看密码验证插件的配置，确保设置已生效。

   ```sql
   SHOW VARIABLES LIKE 'validate_password%';
   ```

这样做之后，你就可以使用低强度的密码了。请注意，降低密码强度可能会增加系统的安全风险，应谨慎操作，并评估其对系统安全性的影响。

如果你有其他关于 MySQL 配置或其他方面的疑问，请随时告诉我。