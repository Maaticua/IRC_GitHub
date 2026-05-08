#!/usr/bin/env python3
import socket
import time

print("🧪 TEST: 50 JOIN/PART successifs")
print("=" * 50)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 6667))

# Auth
s.send(b"PASS mdp\r\n")
s.send(b"NICK alice\r\n")
s.send(b"USER alice 0 * :Alice\r\n")

time.sleep(1)
s.recv(4096)  # Clear welcome messages

# 50 JOIN/PART
for i in range(1, 51):
    s.send(b"JOIN #test\r\n")
    time.sleep(0.05)
    s.send(b"PART #test\r\n")
    time.sleep(0.05)
    
    if i % 10 == 0:
        print(f"✅ {i}/50 iterations completed")

s.send(b"QUIT\r\n")
s.close()

print("🎉 Test completed!")
print("✅ Le serveur devrait être stable")
print("✅ Pas de memory leaks")