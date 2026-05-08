#!/usr/bin/env python3
import socket
import time

print("💬 TEST: 100 messages successifs")
print("=" * 50)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 6667))

# Auth
s.send(b"PASS mdp\r\n")
s.send(b"NICK spammer\r\n")
s.send(b"USER spammer 0 * :Spammer\r\n")

time.sleep(1)
s.recv(4096)

s.send(b"JOIN #spam\r\n")
time.sleep(0.5)
s.recv(4096)

print("📤 Envoi de 100 messages...")

for i in range(1, 101):
    msg = f"PRIVMSG #spam :Message number {i}\r\n"
    s.send(msg.encode())
    time.sleep(0.02)  # 20ms entre chaque message
    
    if i % 20 == 0:
        print(f"✅ {i}/100 messages sent")

s.send(b"QUIT\r\n")
s.close()

print("\n🎉 Test completed!")
print("✅ Le serveur devrait avoir traité tous les messages")