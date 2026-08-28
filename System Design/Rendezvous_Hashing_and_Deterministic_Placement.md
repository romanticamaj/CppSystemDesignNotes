# Rendezvous Hashing：從 Request Distribution 到 Deterministic Placement

## 核心心智模型

在 System Design 裡，「選一台 node」其實有兩種完全不同的問題：

| 問題 | 核心問題 | 常見方法 |
|---|---|---|
| Load / Request Distribution | 這次 request 給誰處理？ | Round Robin、Weighted RR、Least Connections |
| Deterministic Mapping / Placement | 這個 key 應該屬於誰？ | Consistent Hashing、Rendezvous Hashing |

最重要的分界是 **ownership**。

- **Distribution**：A / B / C 都能處理，只要把 workload 分散。
- **Placement**：某個 key / object / partition 應該穩定屬於某個 node。

---

## 1. Request Distribution

典型場景是 stateless API server：

```text
Client
  ↓
Load Balancer
  ↓
┌─────┬─────┬─────┐
│ API │ API │ API │
│  A  │  B  │  C  │
└─────┴─────┴─────┘
        ↓
   Shared DB
```

Round Robin：

```text
request 1 → A
request 2 → B
request 3 → C
request 4 → A
```

同一個 user 下一次不一定還去同一台：

```text
user_123 → A
user_123 → C
user_123 → B
```

因為 server interchangeable，所以目標是：

> 把 request / workload 平均或合理地分散出去。

RR 也可以有 weight，因此「有沒有權重」不是 RR 與 hashing 的根本差別。

---

## 2. Deterministic Mapping / Placement

典型問題：

```text
cache_key  → cache node
user_id    → shard
object_id  → storage node
tenant_id  → backend
```

這裡需要：

```text
placement(key, nodes) → node
```

而且相同條件下結果要穩定：

```text
user_123 → Cache B
user_123 → Cache B
user_123 → Cache B
```

### Mapping vs Placement

- **Mapping**：偏向描述關係，`key → node`
- **Placement**：偏向描述資源應該放在哪裡，例如 partition、object、replica

在 distributed cache、database、distributed storage 裡，通常會談 placement。

---

## 3. 為什麼不能直接 hash(key) % N？

最直覺的做法：

```text
node = hash(key) % N
```

問題在於 N 改變時：

```text
hash(key) % 4
       ↓
hash(key) % 3
```

大量 key 的結果都會改變。

對 distributed cache 來說：

```text
大量 key remap
  ↓
cache miss
  ↓
大量 request 打 DB / backend
  ↓
cache warm-up
  ↓
latency / load 上升
```

這種因 topology / membership 改變，造成大量 cache 失效與重建的現象，可稱為 **cache churn**。

因此 Consistent Hashing 與 Rendezvous Hashing 的共同目的，是：

> node membership 改變時，盡量維持原本的 key → node mapping，做到 minimal remapping / minimal disruption。

---

## 4. Rendezvous Hashing 是什麼？

Rendezvous Hashing 又叫 **Highest Random Weight (HRW) Hashing**。

核心規則：

```text
owner(key) = argmax H(key, node)
```

對同一個 key，和所有 node 各算一次 score：

```text
key = user_123

H(user_123, A) = 128
H(user_123, B) = 921  ← winner
H(user_123, C) = 537
H(user_123, D) = 304
```

所以：

```text
user_123 → B
```

可以把它想成：

> 每個 node 都跟這個 key 比一場，分數最高的人拿走 key。

---

## 5. 為什麼 node 掛掉時只影響少量 key？

原本：

```text
key1:
A = 90  ← winner
B = 30
C = 50

key2:
A = 20
B = 80  ← winner
C = 40

key3:
A = 10
B = 30
C = 70  ← winner
```

mapping：

```text
key1 → A
key2 → B
key3 → C
```

如果 C 掛掉：

```text
key1 → A   不變
key2 → B   不變
key3 → B   remap
```

原因是：

```text
H(key1, A)
H(key1, B)
```

這些 score 都是 deterministic，不會因為 C 消失而改變。

因此：

> 只有原本 winner 是 C 的 key，需要重新找 winner。

加入新 node D 也是同理：只有 D 的 score 超過原 winner 的 key，才會搬到 D。

---

## 6. Rendezvous Hash 的 hash function 怎麼設計？

核心：

```text
score = H(key, node_id)
```

例如：

```text
H("user_123", "cache-A")
H("user_123", "cache-B")
H("user_123", "cache-C")
```

Hash function 需要：

1. **Deterministic**：所有 client / router 對相同輸入算出相同結果
2. **Uniform distribution**：score 分布要夠均勻
3. **Avalanche effect**：輸入小變化，輸出應大幅改變
4. **Fast**：基本 Rendezvous 每次 lookup 會對所有 nodes 計分

通常可使用高速 non-cryptographic hash，例如 xxHash、MurmurHash 等；若有 adversarial input，再考慮 keyed hash / stronger protection。

### Encoding 要避免 ambiguity

不要單純做：

```text
key + node_id
```

因為：

```text
("ab", "c")  → "abc"
("a",  "bc") → "abc"
```

比較安全的做法是使用明確 delimiter、length-prefix 或 structured binary encoding。

### node_id 要穩定

避免直接把容易改變的 IP 當 node identity。

比較好的 node identity：

```text
cache-shard-17
storage-node-42
partition-server-8
```

如果 node identity 改變，Rendezvous 會視為：

```text
remove old node
+
add new node
```

進而產生不必要的 remapping。

---

## 7. Consistent Hashing vs Rendezvous Hashing

### Consistent Hashing

把 node 放到 hash ring：

```text
             A
        /         \
      D             B
        \         /
             C
```

key 也 hash 到 ring 上，再找到順時針下一個 node。

核心模型：

> 先建立全域 hash space，再看 key 落在哪個 node 的區域。

### Rendezvous Hashing

沒有 ring：

```text
score(key, A)
score(key, B)
score(key, C)
score(key, D)
      ↓
    max
```

核心模型：

> key 直接和所有 node 比分數。

### 比較

| | Consistent Hashing | Rendezvous Hashing |
|---|---|---|
| 核心模型 | Hash Ring | Key 與每個 node 計分 |
| 查找 | ring 上找下一個 node | 選最高分 node |
| Node 增減 | 少量 key 移動 | 少量 key 移動 |
| 額外資料結構 | ring / sorted structure | 幾乎不需要 |
| Virtual Node | 常見 | 通常不需要 |
| Top-K replicas | 較繞 | 很自然 |
| 基本 lookup | 約 O(log N) | O(N) |
| 實作複雜度 | 較高 | 較簡單 |

---

## 8. 為什麼 Consistent Hashing 常需要 Virtual Nodes？

少量 physical nodes 在 ring 上可能分布不均：

```text
A: 10%
B: 45%
C: 20%
D: 25%
```

因此常把每個 physical node 放成多個 vnode：

```text
A1 A2 A3 A4 ...
B1 B2 B3 B4 ...
C1 C2 C3 C4 ...
```

用大量 virtual nodes 讓 partition 分布更平均。

Rendezvous Hashing 則讓每個 key 都和所有 node 比賽；如果 hash 品質夠好，unweighted 情況下每個 node 長期勝出的機率自然接近：

```text
1 / N
```

因此通常不需要 vnode。

---

## 9. Rendezvous 的主要 trade-off：O(N)

基本 Rendezvous lookup：

```text
H(key, node1)
H(key, node2)
...
H(key, nodeN)
```

所以是：

```text
O(N)
```

Consistent Hashing 常可做到：

```text
hash(key)
  ↓
binary search ring
  ↓
O(log N)
```

但 O(N) 是否是問題要看 N：

- 幾十個 cache nodes：通常很便宜
- 幾百個 nodes：仍可能可接受
- 幾萬、十萬個 nodes：基本 Rendezvous 就可能太貴

因此不能只看 Big-O，要看 node count、hash cost 與整體 request latency。

---

## 10. Replica placement 是 Rendezvous 很漂亮的地方

如果 replication factor = 3，只要排序分數：

```text
B → 921
C → 537
D → 304
A → 128
```

直接：

```text
primary   → B
replica 1 → C
replica 2 → D
```

因此 Top-K placement 很自然，適合 distributed storage、cache replication 等場景。

Consistent Hashing 通常要沿 ring 繼續找下一個不同 physical node，還要處理 vnode 與 failure domain。

---

## 11. Weighted Rendezvous Hashing

如果 node capacity 不同：

```text
A: 32 cores
B: 8 cores
C: 8 cores
```

可能希望 A 接到更多 key。

這時可以使用 weighted HRW。

但要注意：

> 不能把普通 hash score 直接粗暴地乘上 weight，就假設會得到正確的 proportional allocation。

正式做法會把 hash 轉成合適的隨機變數，再把 node weight 納入 ranking，使節點取得 key 的機率符合 weight。

面試層級通常記住這件事即可：

```text
unweighted HRW:
rank by H(key, node)

weighted HRW:
rank by a weight-aware score transformation
```

---

## 12. RR vs Hashing：不要用「平均 vs 權重」區分

容易講錯的版本：

> RR 是平均取 node，Hashing 有 priority / weight。

更精確的版本：

> RR 是 request-level load distribution；hash-based routing 是 key-level deterministic placement。Consistent / Rendezvous Hashing 進一步處理 membership change 時的 minimal remapping。

因為：

- RR 可以 Weighted Round Robin
- Rendezvous Hashing 在 unweighted 情況下，本來就希望 key 長期接近均勻分布

真正差異是：

```text
RR / Load Balancing
= 誰來處理「這一次 request」？

Hash-based Placement
= 「這個 key」穩定屬於誰？
```

---

## 13. 一個系統裡兩者常同時存在

```text
                  Client
                    ↓
              Load Balancer
                    ↓
           ┌────────┼────────┐
          API A    API B    API C
                    ↓
             Rendezvous Hash
                    ↓
           ┌────────┼────────┐
        Cache 1  Cache 2  Cache 3
```

上半部：

> Request Distribution：誰來處理這次 request？

下半部：

> Deterministic Placement：這個 cache key 屬於哪台？

這是 System Design 很常見的分層。

---

## 14. 面試回答模板

如果題目是：

> 有一個 distributed cache cluster，要把 key 分散到 cache nodes，你怎麼做？

可以回答：

> 我需要的是 deterministic key placement，而不是單純 request distribution。可以用 Consistent Hashing 或 Rendezvous Hashing。若 node 數量不大，我會考慮 Rendezvous Hashing：它不需要維護 hash ring 或 virtual nodes，每個 key 對所有 nodes 計分並選最高分；membership 改變時只有少量 key remap，而且 Top-K replica placement 很自然。代價是基本 lookup 是 O(N)，所以 node 數量非常大時，Consistent Hashing 或其他分層 placement 方法可能更合適。

---

## 最後記住四句

```text
Round Robin
= 這次 request 給誰？
```

```text
Deterministic Placement
= 這個 key 屬於誰？
```

```text
Consistent Hashing
= 把 node 放到 ring 上，看 key 落在哪個區域。
```

```text
Rendezvous Hashing
= 每個 node 都跟 key 比一場，分數最高的人拿走 key。
```

兩種 hashing 的共同價值：

> 在 node membership 改變時，維持穩定的 key → node mapping，降低資料搬移與 cache churn。
