# Case Study: Audio Streaming Media Delivery

## 問題背景

從 WebinarJam 的 replay 頁面觀察到一個典型媒體傳輸架構：

- Browser 直接向 CloudFront 取得 `video/mp4`
- Response header 顯示 `Server: AmazonS3`
- 使用 `206 Partial Content`
- Server 支援 `Accept-Ranges: bytes`

這代表影片本體很可能放在 Object Storage（S3），前面由 CDN（CloudFront）提供，播放器透過 HTTP Range Request 取得需要的 byte range。

這引出一個更廣泛的 System Design 問題：

> Spotify、TIDAL、Apple Music、Calm 等音訊服務，從後端到前端通常如何設計 media delivery？

---

## 核心心智模型：Control Plane vs Media/Data Plane

最重要的拆分是：

> **Control Plane：決定使用者能不能播、可以播什麼。**  
> **Media/Data Plane：真正把 audio bytes 傳到裝置。**

典型架構：

```text
                    Client
                      │
              Control Plane
                      │
        ┌─────────────▼─────────────┐
        │      Application API      │
        │                           │
        │ Auth / User / Catalog     │
        │ Subscription / Entitlement│
        │ Playback Session          │
        └─────────────┬─────────────┘
                      │
              playback credential
                      │
                      ▼

               Media Data Plane

Client ───────────────────────────→ CDN
                                    │
                                    ▼
                              Object Storage
```

Backend 仍然很重要，但通常不應該位於大量 media bytes 的主要傳輸路徑中。

理想情況是：

```text
Backend:
「這個 user 有權播放 Track 123」
        │
        ▼
產生短效 playback credential
        │
        ▼
Client ─────────────→ CDN
                      │
                      ▼
                  audio bytes
```

也就是：

> **Backend 負責 authorization / playback control；CDN 負責 media delivery。**

---

## 一次 Play Request 的典型流程

假設使用者播放 Track 123：

```text
1. Client
   │
   │ Play Track 123
   ▼
2. Playback API
   │
   ├─ authenticated?
   ├─ subscription valid?
   ├─ region allowed?
   ├─ device allowed?
   └─ track licensed?
   │
   ▼
3. Playback Session
   │
   └─ short-lived credential
      media metadata
      codec / bitrate
   │
   ▼
4. Client
   │
   │ GET media
   ▼
5. CDN
   │
   ▼
6. Audio bytes / segments
   │
   ▼
7. Decoder
   │
   ▼
8. Speaker
```

---

# Delivery Protection：從簡單到完整

## Level 0 — Public Object

```text
https://cdn.example.com/song.mp3
```

知道 URL 就能永久存取。

這是最簡單、也最弱的模式。

---

## Level 1 — Authenticated API

```text
Client
  ↓
Backend Auth
  ↓
Backend 回傳 media URL
```

如果 URL 本身是永久公開的，使用者 copy URL 後仍然可以重複使用。

所以只把 authentication 放在 API 層，並不能真正限制 media object。

---

## Level 2 — Signed URL

這是一般 SaaS、podcast、meditation、proprietary audio app 很好的 baseline。

```text
Client
  │
  │ POST /playback/session
  ▼
Backend
  │
  ├─ authenticate user
  ├─ check subscription
  └─ check track permission
  │
  ▼
Generate signed URL
  │
  ▼
https://cdn.example.com/audio/123.m4a
?expires=...
&signature=...
```

Client 直接向 CDN 取得 media。

CDN 負責檢查：

- signature
- expiration

這樣可以避免 Application Server 變成 bandwidth proxy。

---

# HTTP Range Request

如果 media 是一個完整檔案，也不代表 client 一定要一次下載整個檔案。

例如：

```http
GET /audio.m4a
Range: bytes=1000000-2000000
```

Server / CDN 可以回：

```http
HTTP/1.1 206 Partial Content
Content-Range: bytes 1000000-2000000/5000000
```

這樣播放器可以：

- seek
- resume
- 只抓需要的部分
- 避免完整下載後才能播放

因此：

> **完整 object + HTTP Range 本身就是一種合理的 streaming delivery。**

---

# Level 3 — Segmented Media

下一階段可以使用 HLS / DASH：

```text
track/
  manifest.m3u8
  segment-0001.aac
  segment-0002.aac
  segment-0003.aac
```

或：

```text
manifest.mpd
chunk-1.m4s
chunk-2.m4s
chunk-3.m4s
```

但一個很重要的觀念是：

> **Segmentation 主要不是因為檔案大。**

它真正解決的是 playback capability。

---

## Segmentation 的主要價值

### 1. Seek

長音檔可以直接取得指定區段。

### 2. Adaptive Bitrate

根據網路狀況，在 segment boundary 切換 bitrate：

```text
good network  → 256 kbps
poor network  → 128 kbps
bad network   → 64 kbps
```

### 3. Buffer Management

播放器可以維持小量 buffer：

```text
[played][current][buffer][buffer]
```

### 4. Retry / Prefetch / Cache

每個 segment 可以獨立：

- retry
- cache
- prefetch
- expire

### 5. DRM / Content Encryption

Segment-based packaging 也更適合跟 DRM pipeline 整合。

---

# Segmentation 本身不是 Security

只有：

```text
segment-001.m4s
segment-002.m4s
segment-003.m4s
```

並不代表內容被保護。

如果所有 segment 都公開，client 還是可以把全部抓下來重新組合。

真正有意義的是：

```text
segmentation
+
authorization
+
short-lived credential
+
content encryption / DRM
```

---

# Transport Encryption vs Content Encryption

這兩者要分開。

## Transport Encryption

```text
Client ⇄ HTTPS ⇄ Backend
Client ⇄ HTTPS ⇄ CDN
```

HTTPS / TLS 幾乎是現代服務 baseline。

通常由：

- CDN
- Load Balancer
- Edge

處理。

這不需要 application 自己重新設計 encryption。

---

## Content Encryption

這是另一個 layer：

```text
plaintext audio
      ↓
AES encryption
      ↓
encrypted media
      ↓
CDN
```

這才會引入：

- packaging complexity
- key management
- license server
- device compatibility
- DRM

因此：

> **Transport encryption 幾乎一定做；content encryption 才是產品與 threat model 的 trade-off。**

---

# Level 4 — Content Encryption

如果 media 本身加密：

```text
Client ───────→ CDN
                │
                ▼
        encrypted segments
```

但 key 不能跟 media 一起裸放：

```text
encrypted-media
+
public-key-file
```

否則保護沒有意義。

通常需要另外的授權路徑：

```text
Client
  ↓
Key / License Server
  ↓
authorization
  ↓
key / license
```

---

# Level 5 — DRM

對 Spotify、TIDAL、Apple Music 等有高價值授權內容的服務，threat model 不同。

核心問題不是：

> 「不要讓 user 看到 CDN URL。」

因為 browser / app 要播放 media，就一定必須知道去哪裡取得資料。

真正要保護的是：

> **即使 client 看得到 media request，也不能輕易把它轉成永久、可重用、可散布的資產。**

典型 DRM pipeline：

```text
                    Playback API
                         │
                    entitlement
                         │

Client ───────────────→ CDN
  │                     │
  │                     ▼
  │              encrypted segments
  │
  └──────────────────→ DRM License Server
                         │
                         ▼
                 Protected Playback
                         │
                         ▼
                      Speaker
```

這時 media URL 可見並不是主要問題。

主要 protection 在：

- short-lived credential
- encrypted media
- key / license authorization
- protected playback pipeline

---

# Content Value / Threat Model 決定架構

## Calm / Meditation / Podcast 類

如果內容是平台自有：

```text
Auth
+
Playback entitlement
+
Signed CDN URL
+
HTTP Range
```

通常已經是很好的第一版。

如果內容很長或需要 ABR：

```text
+
HLS / DASH
```

不一定需要 DRM。

---

## Spotify / TIDAL / Apple Music 類

核心資產通常是授權音樂。

這些服務更可能需要：

```text
Auth
+
Subscription entitlement
+
Playback session
+
CDN
+
segmented / ranged media
+
content encryption
+
DRM / protected playback
```

因為商業上的 threat model 是：

> 使用者被授權「stream」，不等於取得永久可散布的 master file。

---

# 什麼時候要做 Segmentation？

不要單純用檔案大小判斷。

應該問：

```text
需要 seek 嗎？
需要 adaptive bitrate 嗎？
需要精細 buffer control 嗎？
需要 DRM 嗎？
```

如果答案都是 No：

```text
3 分鐘音樂
AAC / Opus / M4A
Signed URL
CDN
HTTP Range
```

通常已經非常夠用。

如果是：

- 1 小時 meditation
- 2 小時 DJ set
- podcast
- audiobook
- video

那 HLS / DASH 的價值就會開始變大。

因此：

> **HLS / DASH 是 playback capability decision，不只是 file size optimization。**

---

# Ingest Pipeline

完整系統不能只看播放端。

在 playback 之前，通常還有 offline ingest pipeline：

```text
Master Audio
    │
    ▼
Upload
    │
    ▼
Validation
    │
    ▼
Transcoding
  /   |    \
AAC  Opus  FLAC
64k  128k  Lossless
    │
    ▼
Packaging
    │
    ├─ whole object
    │
    └─ HLS / DASH segments
    │
    ▼
Encryption (optional)
    │
    ▼
Object Storage
    │
    ▼
CDN
```

---

# 完整 Audio Streaming System Design

```text
                  OFFLINE / INGEST

Master
  ↓
Upload
  ↓
Validate
  ↓
Transcode
  ↓
Package
  ↓
Encrypt (optional)
  ↓
Object Storage
  ↓
CDN


                  ONLINE / PLAYBACK

Client
  │
  ├────→ Auth / Catalog API
  │
  ├────→ Playback API
  │         ↓
  │      entitlement
  │         ↓
  │      playback token
  │
  ├──────────────────→ CDN
  │                    ↓
  │                  media
  │
  └────→ License Server   ← DRM 才需要
```

---

# Architecture Evolution

一個合理的演進順序：

```text
V1
Public URL
    ↓
V2
Auth + Signed URL
    ↓
V3
CDN + HTTP Range
    ↓
V4
HLS / DASH
    ↓
V5
Content Encryption
    ↓
V6
DRM / License Server
```

每一層都是針對新的 requirement，而不是一開始全部做滿。

---

# 面試時的推薦回答

如果在 L5 System Design 面試設計 Audio Streaming Service，可以先說：

> 我會先把 metadata/control path 和 media delivery path 分開。Application backend 負責 authentication、entitlement 與產生短效 playback credential；audio object 放在 object storage，透過 CDN delivery。第一版我會使用 signed URL + HTTP Range。當需求出現 adaptive bitrate、長內容或 DRM 時，再演進成 HLS/DASH segmentation 與 encrypted media。

這裡真正展現的是：

> **從 requirement 推 architecture，而不是直接複製 Spotify 的完整架構。**

---

# 關鍵句

> **Media URL visibility is unavoidable; media reusability is what we actually protect.**

> **Backend controls playback; CDN delivers bytes.**

> **Segmentation is a playback capability decision, not merely a file-size optimization.**

> **Transport encryption is baseline; content encryption is a threat-model decision.**

> **Control Plane decides whether you may play. Data Plane delivers what you play.**
