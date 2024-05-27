import pickle

# 一个不安全的反序列化函数
def unsafe_deserialize(data):
    return pickle.loads(data)

# 模拟从不受信任来源接收的恶意数据
malicious_data = b"cos\nsystem\n(S'echo vulnerable'\ntR."

# 反序列化恶意数据
unsafe_deserialize(malicious_data)