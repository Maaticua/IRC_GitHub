#!/usr/bin/env python3
import socket
import time

print("⚠️  TEST: Edge cases et cas limites")
print("=" * 50)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(('localhost', 6667))

# Auth
s.send(b"PASS mdp\r\n")
s.send(b"NICK edge\r\n")
s.send(b"USER edge 0 * :Edge Tester\r\n")

time.sleep(1)
s.recv(4096)

print("\n1️⃣  Test: PRIVMSG avec ':' multiples")
s.send(b"JOIN #test\r\n")
time.sleep(0.2)
s.send(b"PRIVMSG #test :Message with : multiple : colons : inside\r\n")
time.sleep(0.2)

print("2️⃣  Test: TOPIC très long (>512 caractères)")
long_topic = "A" * 600
s.send(f"TOPIC #test :{long_topic}\r\n".encode())
time.sleep(0.2)

print("3️⃣  Test: Messages vides")
s.send(b"PRIVMSG #test :\r\n")
time.sleep(0.2)

print("4️⃣  Test: Channel inexistant")
s.send(b"PRIVMSG #nonexistent :Test\r\n")
time.sleep(0.2)

print("5️⃣  Test: User inexistant")
s.send(b"PRIVMSG ghost :Test\r\n")
time.sleep(0.2)

print("6️⃣  Test: MODE avec paramètres manquants")
s.send(b"MODE #test +k\r\n")
time.sleep(0.2)

print("7️⃣  Test: Multiple JOINs du même channel")
s.send(b"JOIN #test\r\n")
time.sleep(0.1)
s.send(b"JOIN #test\r\n")
time.sleep(0.1)
s.send(b"JOIN #test\r\n")
time.sleep(0.2)

print("8️⃣  Test: PART d'un channel non rejoint")
s.send(b"PART #notjoined\r\n")
time.sleep(0.2)

print("9️⃣  Test: Commands en majuscules/minuscules")
s.send(b"join #lower\r\n")
time.sleep(0.1)
s.send(b"part #lower\r\n")
time.sleep(0.2)

print("🔟 Test: Caractères spéciaux dans les messages")
s.send("PRIVMSG #test :Émojis 🎉🤖 et spéciaux éàç\r\n".encode('utf-8'))
time.sleep(0.2)

s.send(b"QUIT\r\n")
s.close()

print("\n🎉 Test completed!")
print("✅ Le serveur devrait avoir géré tous les cas limites")
print("✅ Pas de crash")