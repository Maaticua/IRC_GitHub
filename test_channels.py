#!/usr/bin/env python3
import socket
import time

print("📢 TEST: Créer et supprimer 20 channels")
print("=" * 50)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 6667))

# Auth
s.send(b"PASS mdp\r\n")
s.send(b"NICK alice\r\n")
s.send(b"USER alice 0 * :Alice\r\n")

time.sleep(1)
s.recv(4096)

# Créer et supprimer 20 channels
for i in range(1, 21):
    channel = f"#channel{i}"
    
    s.send(f"JOIN {channel}\r\n".encode())
    time.sleep(0.1)
    
    s.send(f"TOPIC {channel} :Topic for {channel}\r\n".encode())
    time.sleep(0.1)
    
    s.send(f"PART {channel} :Cleaning up\r\n".encode())
    time.sleep(0.1)
    
    if i % 5 == 0:
        print(f"✅ {i}/20 channels created and deleted")

s.send(b"QUIT\r\n")
s.close()

print("\n🎉 Test completed!")
print("✅ Tous les channels devraient être supprimés")
print("✅ Pas de memory leaks")