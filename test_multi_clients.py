#!/usr/bin/env python3
import socket
import threading
import time

print("👥 TEST: 10 clients simultanés")
print("=" * 50)

def client_thread(client_id):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 6667))
        
        nick = f"user{client_id}"
        
        s.send(b"PASS mdp\r\n")
        s.send(f"NICK {nick}\r\n".encode())
        s.send(f"USER {nick} 0 * :{nick}\r\n".encode())
        
        time.sleep(0.5)
        s.recv(4096)
        
        s.send(b"JOIN #multi\r\n")
        time.sleep(0.3)
        
        for i in range(10):
            s.send(f"PRIVMSG #multi :Message from {nick} #{i}\r\n".encode())
            time.sleep(0.1)
        
        time.sleep(1)
        
        s.send(b"PART #multi\r\n")
        time.sleep(0.2)
        
        s.send(b"QUIT\r\n")
        s.close()
        
        print(f"✅ Client {client_id} completed")
    except Exception as e:
        print(f"❌ Client {client_id} error: {e}")

threads = []

print("🚀 Lancement de 10 clients...")

for i in range(10):
    t = threading.Thread(target=client_thread, args=(i,))
    t.start()
    threads.append(t)
    time.sleep(0.2)  # Petit délai entre chaque client

for t in threads:
    t.join()

print("\n🎉 Test completed!")
print("✅ Tous les clients devraient s'être déconnectés proprement")
print("✅ Le serveur devrait être stable")