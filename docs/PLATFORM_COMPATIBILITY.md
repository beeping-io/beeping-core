# 📡 Platform Compatibility — Beeping Core

> Estado de compatibilidad de `beeping-core` con plataformas de audio del mundo
> real. Se actualiza cada vez que se verifica un nuevo sample rate o plataforma.
>
> Ultima actualizacion: 2026-04-13

---

## 🎵 Musica & Streaming

| Plataforma | Rate | Estado |
|---|---|---|
| 🟢 Spotify HiFi | 96000 | 🟢 |
| 🟠 Spotify (normal) | 44100 | 🟢 |
| 🍎 Apple Music Lossless | 96000 | 🟢 |
| 🍎 Apple Music (normal) | 44100 | 🟢 |
| 🌊 Tidal HiFi | 96000 | 🟢 |
| 🟡 Amazon Music HD | 96000 | 🟢 |
| ☁️ SoundCloud | 44100 | 🟢 |
| 🎸 Bandcamp | 44100 | 🟢 |

## 📱 Redes Sociales — Video

| Plataforma | Rate | Estado |
|---|---|---|
| ▶️ YouTube | 48000 | 🟢 |
| 🎵 TikTok | 48000 | 🟢 |
| 📸 Instagram Reels | 48000 | 🟢 |
| 📘 Facebook Video / Reels | 48000 | 🟢 |
| 💼 LinkedIn Video | 48000 | 🟢 |
| 🟣 Twitch | 48000 | 🟢 |
| ❌ X (Twitter) Video | 48000 | 🟢 |
| 📌 Pinterest Video | 48000 | 🟢 |

## 🎙️ Podcasts

| Plataforma | Rate | Estado |
|---|---|---|
| 🟢 Spotify Podcasts | 48000 | 🟢 |
| 🍎 Apple Podcasts | 44100 | 🟢 |
| 🎧 Google Podcasts | 48000 | 🟢 |
| 🔊 Amazon Music Podcasts | 48000 | 🟢 |

## 💬 Voice Messaging

| Plataforma | Rate | Estado |
|---|---|---|
| 💬 WhatsApp Voice | 16000 | 🔴 |
| ✈️ Telegram Voice | 16000 | 🔴 |
| 💬 Facebook Messenger Voice | 16000 | 🔴 |
| 💼 LinkedIn Voice Messages | 16000 | 🔴 |
| 🎤 Discord Voice | 48000 | 🟢 |

## 📞 Videollamadas

| Plataforma | Rate | Estado |
|---|---|---|
| 📹 Zoom | 48000 | 🟢 |
| 👥 Microsoft Teams | 48000 | 🟢 |
| 📞 Google Meet | 48000 | 🟢 |
| 📱 FaceTime | 48000 | 🟢 |
| 🌐 WebRTC (generico) | 48000 | 🟢 |

## 📺 Broadcast & TV

| Plataforma | Rate | Estado |
|---|---|---|
| 📺 Netflix | 48000 | 🟢 |
| 🏰 Disney+ | 48000 | 🟢 |
| 📡 FM Radio Digital (DAB/DAB+) | 32000 | 🟡 Audible only |
| 📻 Radio por Internet | 32000 | 🟡 Audible only |
| 📺 Cable TV USA (audio tracks) | 24000 | 🟡 Audible only |
| 📡 IPTV | 24000 | 🟡 Audible only |

## 🎹 Produccion Musical (DAWs)

| Plataforma | Rate | Estado |
|---|---|---|
| 🎹 Logic Pro | 96000 | 🟢 |
| 🔴 Ableton Live | 96000 | 🟢 |
| 🎚️ Pro Tools | 96000 | 🟢 |
| 🎵 FL Studio | 96000 | 🟢 |
| 🔊 Cubase | 96000 | 🟢 |

## 📱 Dispositivos Moviles (captura nativa)

| Plataforma | Rate | Estado |
|---|---|---|
| 🍎 iOS (microfono) | 48000 | 🟢 |
| 🤖 Android (microfono) | 48000 | 🟢 |

---

## 📊 Resumen por Sample Rate

| Rate | Audible | Inaudible | All | Plataformas clave |
|---|---|---|---|---|
| **96000** | 🟢 | 🟢 | 🟢 | Musica hi-res, DAWs |
| **48000** | 🟢 | 🟢 | 🟢 | Moviles, video social, videollamadas, podcasts |
| **44100** | 🟢 | 🟢 | 🟢 | CD, web audio, musica streaming |
| **32000** | 🟢 | 🔴 | 🔴 | FM radio digital (solo modo Audible) |
| **24000** | 🟢 | 🔴 | 🔴 | Cable TV (solo modo Audible) |
| **22050** | 🟢 | 🔴 | 🔴 | Legacy low-quality |
| **16000** | 🔴 | 🔴 | 🔴 | Voice messaging — Nyquist 8kHz insuficiente |

### Notas

- **Inaudible** usa frecuencias 17.8-21kHz. A rates <44.1kHz, Nyquist corta
  estas frecuencias (limitacion fisica, no de software).
- **Audible** usa 3.3-10kHz. Funciona hasta 22050 Hz (Nyquist 11kHz).
  A 16kHz (Nyquist 8kHz), las frecuencias superiores del modo se pierden.
- **32k/24k con modo Audible**: funciona pero los tonos son audibles para
  el usuario. Util para broadcast donde el "beep" es aceptable.

---

## 🧪 Metodo de verificacion

Cada resultado requiere **round-trip test exitoso**: encode → decode con
payload `"123456789"` recuperado bit-a-bit. Tests en `tests/` y verificados
con el rate sweep tool.

Modos verificados: `AUDIBLE`, `INAUDIBLE`, `ALL`.
