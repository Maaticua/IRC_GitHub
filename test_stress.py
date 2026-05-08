#!/usr/bin/env python3
import socket
import threading
import time

print("🔥 TEST: CHARGE MAXIMALE")
print("=" * 50)
print("⚠️  Ce test va stresser le serveur!")
print()

def heavy_client(client_id):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 6667))

        nick = f"stress{client_id}"

        s.send(b"PASS mdp\r\n")
        s.send(f"NICK {nick}\r\n".encode())
        s.send(f"USER {nick} 0 * :{nick}\r\n".encode())

        time.sleep(0.3)
        s.recv(4096)

        # 20 JOIN/PART rapides
        for i in range(20):
            s.send(f"JOIN #stress{i % 5}\r\n".encode())
            time.sleep(0.02)
            s.send(f"PRIVMSG #stress{i % 5} :Stress test {i}\r\n".encode())
            time.sleep(0.02)
            s.send(f"PART #stress{i % 5}\r\n".encode())
            time.sleep(0.02)

        s.send(b"QUIT\r\n")
        s.close()

        print(f"✅ Stress client {client_id} done")
    except Exception as e:
        print(f"❌ Client {client_id} crashed: {e}")

print("🚀 Lancement de 20 clients en stress test...")

threads = []
for i in range(20):
    t = threading.Thread(target=heavy_client, args=(i,))
    t.start()
    threads.append(t)
    time.sleep(0.05)

for t in threads:
    t.join()

print("\n🎉 Stress test completed!")
print("✅ Si le serveur est encore vivant, c'est un succès!")