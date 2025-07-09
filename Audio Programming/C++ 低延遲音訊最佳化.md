# **C++ 低延遲音訊處理的架構與技術**

---

## **第一部分：即時音訊的基礎**

在深入探討 C++ 低延遲音訊程式設計的複雜技術之前，必須先建立一個穩固的基礎，理解其核心挑戰與基本原則。本部分將闡述數位音訊領域中延遲的本質、即時系統的嚴苛要求，以及在這種環境下進行軟體開發所必須遵守的基本規則。這些基礎概念是後續所有高階技術與架構設計的基石。

### **第一節：數位音訊中的延遲挑戰**

#### **1.1. 定義延遲：從 ADC 到 DAC 的訊號路徑**

延遲（Latency），在數位音訊的脈絡中，指的是從訊號進入系統到其離開系統之間所經過的時間。這條路徑始於類比訊號（如麥克風捕捉的聲音），終於類比訊號（如喇叭播放的聲音），中間經歷了一連串的數位轉換與處理。理解這條完整的訊號路徑是掌握延遲來源的第一步 1。

典型的訊號路徑包含以下幾個關鍵階段：

1. **類比輸入（Analog Input）**：麥克風或樂器產生連續的類比電氣訊號。
2. **類比至數位轉換器（Analog-to-Digital Converter, ADC）**：音效卡或音訊介面的 ADC 將連續的類比訊號轉換為離散的數位樣本。這個過程包含三個核心步驟：
   - **取樣（Sampling）**：以固定的取樣率（Sampling Rate），例如 44.1 kHz，對類比訊號進行快照，將其轉換為時間上離散的序列。根據奈奎斯特-香農取樣定理（Nyquist-Shannon sampling theorem），取樣率必須至少是訊號最高頻率的兩倍，才能無失真地重建原始訊號 3。
   - **量化（Quantization）**：將每個取樣點的連續振幅對應到一組有限的離散數值。ADC 的解析度（以位元數表示，如 16-bit 或 24-bit）決定了離散層級的數量，這會引入量化誤差（Quantization Error）3。
   - **數位編碼（Digital Coding）**：將量化後的數值轉換為二進位碼，形成電腦可處理的數位音訊資料 3。
3. **輸入緩衝區（Input Buffer）**：ADC 產生的數位樣本被寫入音效卡的硬體輸入緩衝區 2。
4. **CPU 處理（CPU Processing）**：作業系統的音訊驅動程式將資料從硬體緩衝區複製到軟體緩衝區，應用程式在此對音訊資料進行處理，例如加上效果器、混音或合成。
5. **輸出緩衝區（Output Buffer）**：處理完成的音訊資料被寫入軟體輸出緩衝區，再由驅動程式複製到硬體輸出緩衝區。
6. **數位至類比轉換器（Digital-to-Analog Converter, DAC）**：音效卡的 DAC 將數位樣本轉換回連續的類比電氣訊號 4。
7. **類比輸出（Analog Output）**：轉換後的類比訊號驅動喇叭或耳機發出聲音。

總延遲是上述所有階段延遲的總和，主要可分為三類：硬體延遲（ADC/DAC 轉換所需的時間）、驅動程式延遲（資料在硬體與軟體間傳輸的時間），以及軟體處理延遲。在互動式應用中，軟體處理延遲通常是最大且最可控的變數，而它的大小主要由緩衝區的大小決定 1。

#### **1.2. 即時音訊回呼：系統的心跳**

現代作業系統的音訊 API（如 Windows 的 ASIO、WASAPI，macOS 的 CoreAudio，以及 Linux 的 ALSA 或 JACK）大多採用回呼（Callback）模型來處理音訊 4。其運作方式如下：作業系統或音訊驅動程式會以固定且高頻率的間隔，呼叫一個由應用程式開發者提供的函式，這個函式通常被稱為「音訊回呼」或「處理區塊」（Process Block）。

這個回呼函式是即時音訊系統的心臟。每次被呼叫時，它會接收一個包含新輸入音訊樣本的緩衝區，並被要求填滿一個空的輸出緩衝區。這個過程必須在一個極其嚴格的時間限制內完成，這個限制被稱為「處理截止期限」（Processing Deadline）。這個截止期限通常等於一個緩衝區所能容納的音訊時長。例如，在 44.1 kHz 的取樣率下，若緩衝區大小為 256 個樣本，則處理截止期限約為 256/44100≈5.8 毫秒 4。

如果回呼函式未能在截止期限內完成其工作並填滿輸出緩衝區，音效卡將沒有新的資料可以播放，導致音訊流中斷，產生被稱為「爆音」、「喀噠聲」或「音訊下溢」（Underrun/Glitch）的聽覺瑕疵 2。為了確保系統的穩定性，音訊回呼通常在一個獨立於主應用程式執行緒（例如處理使用者介面或檔案操作的執行緒）的高優先級即時執行緒上執行 8。這種執行緒分離的架構，正是後續討論中許多並行挑戰的根源。

#### **1.3. 即時程式設計的最高原則：「時間不等人」**

即時音訊程式設計的核心原則可以精煉為一句話：「如果你不知道它需要花費多長時間，就不要在即時執行緒做這件事」（If you don't know how long it will take, don't do it）4。這條黃金法則禁止在音訊回呼中執行任何執行時間不可預測或無上限的操作。

在通用計算中，我們經常追求「平均情況」下的高效能。一個演算法在 99.9% 的情況下都很快，但偶爾會出現一次長時間的執行，這在許多應用中是可以接受的。然而，在即時音訊領域，唯一重要的指標是「最壞情況」下的執行時間 4。因為即便是單一一次的執行時間超限，也會導致一次可聞的音訊瑕疵，從而破壞使用者的體驗。這種思維模式的轉變，是從「吞吐量導向」（throughput-oriented）的程式設計轉向「延遲導向」（latency-oriented）或「截止期限導向」（deadline-oriented）的程式設計。

#### **1.4. 「禁忌清單」：即時安全 C++ 的實踐指南**

基於上述最高原則，社群與業界專家們總結出了一份在音訊回呼中應極力避免的操作清單。這份「禁忌清單」不僅是效能最佳化的建議，更是確保即時系統正確性的基本要求 4。

**即時音訊回呼中的禁忌操作：**

1. **禁止動態記憶體配置與釋放**：避免使用 new、delete、malloc、free。這些操作可能觸發作業系統呼叫、內部鎖定，並且執行時間具有高度的非確定性 4。即使是看似無害的 C++ 特性，如  
   std::any，也可能隱藏了堆積記憶體配置，應當避免 11。所有需要的記憶體都應在即時迴圈開始前預先配置好。
2. **禁止使用鎖**：避免使用 std::mutex、CriticalSection 等任何形式的鎖定機制。這是導致「優先級反轉」（Priority Inversion）的主要原因，會使高優先級的音訊執行緒被低優先級執行緒無限期阻塞，是即時系統的致命傷 4。
3. **禁止任何 I/O 操作**：避免讀寫檔案、網路通訊端，甚至是標準控制台輸出（如 printf、std::cout）。這些操作速度緩慢且可能阻塞執行緒 4。若需要即時安全的日誌記錄，必須採用特殊設計，例如將日誌訊息放入無鎖佇列中，由另一個非即時執行緒處理寫入 13。
4. **禁止阻塞的作業系統或 API 呼叫**：任何可能等待外部事件或資源的函式都應被禁止 4。
5. **禁止執行時間不穩定的程式碼**：許多標準函式庫的容器或演算法可能在內部進行記憶體重新配置，或其實作具有複雜且非確定性的時間複雜度，應避免在回呼中使用 4。
6. **禁止呼叫不受信任的程式碼**：除非能百分之百確定一個函式庫或函式嚴格遵守以上所有規則，否則不應在音訊回呼中呼叫它 4。

這份「禁忌清單」的深層意義在於，它揭示了即時音訊程式設計不僅僅是追求「速度」，更是追求「確定性」（Determinism）。它迫使開發者從根本上改變程式設計範式。通用 C++ 中常用的工具和模式，在即時音訊執行緒這個高度受限的環境中變得不可用，使其更像是在通用作業系統上運行的嵌入式系統。開發者不能簡單地將一個通用演算法「最佳化」後用於音訊執行緒，而必須從頭開始，圍繞這些限制來設計演算法和資料結構。這也解釋了為何像無鎖佇列（Lock-free Queue）和生產者-消費者模式（Producer-Consumer Pattern）等專門技術，對於穩定的低延遲音訊系統而言，並非可有可無的奢侈品，而是不可或缺的基礎設施。這份禁忌清單，正是推導出後續所有低延遲設計模式的公理集合。

### **第二節：核心緩衝策略**

緩衝（Buffering）是數位音訊串流的核心機制，它在平滑資料流、應對處理時間波動方面扮演著關鍵角色。然而，緩衝策略的選擇直接影響到系統的兩個核心指標：延遲與穩定性。

#### **2.1. 基本權衡：延遲 vs. 穩定性**

在低延遲音訊設計中，開發者面臨一個永恆的權衡：緩衝區大小。這個選擇直接決定了系統的延遲和穩定性 5。

- **大緩衝區（Larger Buffers）**：提供更長的處理截止期限。例如，一個 2048 個樣本的緩衝區在 96 kHz 取樣率下，給予了 CPU 超過 20 毫秒的時間來完成音訊處理 6。這使得系統對 CPU 負載的突發性尖峰更具韌性，能有效防止音訊下溢。然而，其代價是更高的端到端延遲。對於非互動式應用（如音樂播放器），高延遲通常是可以接受的。
- **小緩衝區（Smaller Buffers）**：提供更低的延遲，這對於互動式應用至關重要。例如，在虛擬樂器或現場效果器中，使用者按下按鍵或轉動旋鈕後，期望能立即聽到聲音的變化。低至 1-5 毫秒的延遲通常被認為是理想的 4。然而，小緩衝區意味著更短的處理截止期限和更頻繁的回呼，這不僅增加了 CPU 的總體負載，也讓系統對任何微小的處理延遲都變得極為敏感，從而大大增加了音訊下溢的風險 5。

延遲的大小與緩衝區大小成正比，並且在整個訊號路徑中，由軟體緩衝區引入的延遲往往是佔比最大且最可控的部分 5。因此，選擇合適的緩衝區大小是低延遲設計的第一個關鍵決策。

#### **2.2. 多重緩衝機制**

為了在延遲和穩定性之間尋求更好的平衡，開發者通常會採用多重緩衝（Multi-Buffering）機制，而不僅僅是單一的緩衝區。

- **雙重緩衝（Double Buffering）**：這是最經典的串流模型。系統使用兩個緩衝區：當音訊硬體正在播放其中一個緩衝區（稱為「前緩衝區」，Front Buffer）的內容時，CPU 同時在填充另一個緩衝區（稱為「後緩衝區」，Back Buffer）。當前緩衝區播放完畢後，兩者角色互換。這個機制確保了音訊資料的連續流動，避免了在處理過程中直接修改正在播放的資料而導致的音訊撕裂（Tearing）14。在一個由兩個 50 毫秒緩衝區組成的系統中，CPU 有 50 毫秒的截止期限來填充下一個緩衝區 5。
- **三重緩衝（Triple Buffering）**：在雙重緩衝的基礎上增加第三個緩衝區。其工作流程為：一個緩衝區正在播放，一個緩衝區正在被 CPU 填充，而第三個緩衝區則已填充完畢，處於「已就緒」狀態，等待播放。這種機制的優勢在於應對 CPU 高負載或處理時間不均勻的情況。如果 CPU 處理某個區塊的時間偶爾超過了一個緩衝區的時長，三重緩衝可以提供一個額外的緩衝墊，確保在硬體需要新資料時，總有一個已就緒的緩衝區可以立即提供，從而避免了因處理延遲導致的下溢。這種類比於圖形學中的三重緩衝，後者用於平滑影格率波動 16。然而，這種穩定性的代價是延遲的增加，通常會將基礎延遲加倍 17。例如，某些音訊驅動程式可能在緩衝週期的末尾才觸發回呼，有效縮短了可用的處理時間。在這種情況下，三重緩衝可以確保系統有一個完整的緩衝週期來準備下一份資料，從而提高穩定性 17。
- **N-緩衝（N-Buffering）**：這個概念可以推廣到任意 N 個緩衝區。一個重要的觀察是，使用更多數量的小緩衝區，可以在總延遲相同的情況下，提供比少量大緩衝區更長的有效處理截止期限 5。  
  案例分析 5：
  - **情境 A（雙重緩衝）**：2 個 50 毫秒的緩衝區，總延遲為 100 毫秒。當一個 50 毫秒的緩衝區正在播放時，CPU 必須在 **50 毫秒**內填充另一個緩衝區。
  - **情境 B（N-緩衝）**：10 個 10 毫秒的緩衝區，總延遲同樣為 100 毫秒。當一個 10 毫秒的緩衝區正在播放時，還有 8 個緩衝區在佇列中等待播放。這意味著 CPU 有長達 **90 毫秒**（1×10ms (playing)+8×10ms (queued) 的剩餘時間）的截止期限來填充最後一個空的緩衝區。

顯然，情境 B 為 CPU 提供了更寬裕的處理時間，使其能更好地應對處理負載的突發性尖峰，從而顯著提升系統的穩定性，而總延遲並未增加。

#### **2.3. 環形緩衝區（Ring Buffers）**

環形緩衝區，也稱為循環緩衝區（Circular Buffer），是實現上述所有串流 I/O 和執行緒間通訊的基礎資料結構 5。它本質上是一個固定大小的線性陣列，但被當作首尾相連的環來使用。

其管理通常通過兩個指標或索引來完成：一個讀取指標（Read Pointer/Tail）和一個寫入指標（Write Pointer/Head）。

- 當生產者（Producer）需要寫入資料時，它會將資料放入寫入指標所指向的位置，然後將寫入指標向前移動。
- 當消費者（Consumer）需要讀取資料時，它會從讀取指標所指向的位置讀取資料，然後將讀取指標向前移動。

為了防止資料被覆蓋或讀取到無效資料，必須進行邊界檢查。一個簡單的判斷規則是：當寫入指標的下一個位置不等於讀取指標時（write_ptr \+ 1\!= read_ptr，考慮環繞），表示緩衝區未滿，可以寫入；當讀取指標不等於寫入指標時（read_ptr\!= write_ptr），表示緩衝區不為空，可以讀取 14。

環形緩衝區的高效性和固定大小的特性使其成為實現高效能、即時安全的佇列的理想選擇，特別是後續將深入探討的無鎖佇列。例如，知名的音訊開發框架 JUCE 中的 AbstractFifo 類別，就是一個專門用於管理環形緩衝區索引的控制器，它本身不擁有記憶體，而是為外部的資料緩衝區提供線程安全的讀寫索引管理，這是一個非常清晰且高效的設計模式 18。

緩衝策略的選擇，從根本上決定了一個即時音訊系統架構的複雜程度。一個高延遲、大緩衝區的系統（例如使用簡單的雙重緩衝）對 CPU 的處理時間非常寬容，其截止期限較長。在這種情況下，開發者或許可以使用較為傳統的、基於鎖的同步機制，因為偶爾的阻塞可能不會導致錯過截止期限。

然而，一旦專案的目標轉向追求極低的延遲（例如低於 5 毫秒），這就意味著必須使用許多小緩衝區的策略。這種選擇會產生極其嚴苛的處理截止期限，使得系統對任何形式的非確定性都變得極度敏感 4。這種敏感性產生了連鎖反應，迫使開發者必須採用本報告後續章節中討論的各種高階技術。它要求使用無鎖資料結構來徹底避免優先級反轉；它要求設定即時執行緒優先級來防止被其他無關緊要的系統任務搶佔；它甚至要求使用 CPU 親和性來將執行緒綁定到特定核心，以最小化由快取失效和執行緒遷移引起的時脈抖動（Jitter）。

因此，一個專案的延遲目標不僅僅是一個效能指標，它是一個根本性的架構驅動因素。試圖僅僅通過調整緩衝區大小來實現低延遲，而不解決其背後所必需的架構支撐問題，是導致系統充滿難以除錯的音訊瑕疵的普遍原因。這也解釋了為何一些開發者在低延遲之路上步履維艱，而另一些開發者卻能構建出穩定可靠的高效能系統——後者認識到，低延遲是一個需要從 C++ 語言子集選擇、資料結構設計，到作業系統層面干預的整體性工程。

---

## **第二部分：並行與資料交換**

在現代音訊應用中，即時音訊執行緒很少孤立運行。它需要與其他非即時執行緒（如處理使用者介面、網路通訊或檔案 I/O 的執行緒）進行頻繁的資料交換。由於「禁忌清單」的存在，這種跨執行緒通訊成為低延遲設計中最核心的挑戰之一。本部分將探討解決這一挑戰的關鍵架構模式與技術，從傳統的鎖定方法過渡到即時系統所必需的無鎖程式設計。

### **第三節：生產者-消費者架構模式**

#### **3.1. 解耦即時執行緒**

核心問題在於：即時音訊執行緒（作為「消費者」）需要從其他執行緒（作為「生產者」）獲取資料或指令，例如來自 GUI 的參數變更、從磁碟載入的音訊樣本，或是網路傳來的事件 8。直接的函式呼叫或共享變數存取是不可行的，因為這會違反「禁忌清單」中的規則（例如，UI 執行緒可能持有鎖，導致音訊執行緒阻塞）。

生產者-消費者模式（Producer-Consumer Pattern）提供了一個優雅的解決方案。它通過引入一個共享的中介資料結構（通常是一個佇列或緩衝區），來解耦生產者執行緒和消費者執行緒 23。

- **生產者（Producer）**：非即時執行緒（如 UI 執行緒）將需要傳遞的資料或命令（例如，「將增益參數設定為 0.5」）打包成一個訊息，並將其放入共享佇列中。這個操作可以在不直接干擾音訊執行緒的情況下完成。
- **消費者（Consumer）**：即時音訊執行緒在其回呼函式中，以一種非阻塞的方式檢查佇列。如果佇列中有新的訊息，它會取出並處理這些訊息（例如，更新內部的增益參數）。如果佇列為空，它會直接跳過，繼續處理音訊，而不會被阻塞。

這種模式將時間敏感的音訊處理與時間不敏感的外部事件處理完全分開，是構建響應式、穩定的音訊應用的基礎架構。

#### **3.2. 基於鎖的實現：一個不適用於即時的範例**

對於通用目的的多執行緒程式設計，生產者-消費者模式的一個典型實現是使用 std::queue，並通過 std::mutex 和 std::condition_variable 來保護共享佇列的存取 23。

一個簡化的實現可能如下：

- 生產者執行緒首先鎖定互斥鎖（std::mutex），然後將資料推入佇列，接著解鎖互斥鎖，並使用條件變數（std::condition_variable）的 notify_one() 方法來喚醒可能正在等待的消費者執行緒。
- 消費者執行緒也首先鎖定互斥鎖。如果佇列為空，它會在使用條件變數的 wait() 方法上等待，這會自動釋放鎖並使執行緒進入休眠狀態，直到被生產者喚醒。當被喚醒後，它會重新獲取鎖，從佇列中取出資料，最後解鎖。

**分析**：儘管這種基於鎖的實現在功能上是正確的，並且在許多通用場景中非常有用，但它**完全不適用於即時音訊處理**。其根本缺陷在於，消費者（即高優先級的音訊執行緒）可能會因為 wait() 或嘗試鎖定一個已被低優先級生產者執行緒持有的互斥鎖而**被阻塞**。這就是典型的**優先級反轉**（Priority Inversion）場景，它會導致音訊執行緒錯過其處理截止期限，產生音訊下溢 8。因此，我們必須尋求一種不依賴鎖的通訊機制。

### **第四節：無鎖程式設計：通往確定性之路**

無鎖程式設計（Lock-Free Programming）是解決即時執行緒間通訊挑戰的關鍵。它旨在建立一種即使在多執行緒爭用的情況下，也能保證整個系統持續取得進展的同步機制，從而避免傳統鎖帶來的阻塞和非確定性延遲。

#### **4.1. 鎖的危害：優先級反轉詳解**

優先級反轉是即時系統中的一個經典且致命的問題。當一個高優先級任務（如音訊執行緒）需要存取一個被低優先級任務（如 UI 執行緒）持有的共享資源（如互斥鎖）時，高優先級任務將被迫進入等待狀態。更糟糕的是，作業系統的排程器可能因為存在一個中等優先級的任務正在運行，而不會給予那個持有鎖的低優先級任務足夠的 CPU 時間來完成其工作並釋放鎖。結果就是，高優先級任務被一個不相關的中等優先級任務無限期地「間接」阻塞，最終導致其錯過截止期限 8。

無鎖程式設計通過完全避免使用鎖來根除這個問題。其核心思想是利用 CPU 提供的原子操作（Atomic Operations）來協調對共享資料的存取。在無鎖的定義下，系統整體總能取得進展。一個更強的保證是**無等待**（Wait-Free），它保證每個執行緒都能在有限的步驟內完成其操作，而不管其他執行緒的狀態如何 8。對於音訊回呼與單一 UI 執行緒之間的通訊，一個無等待的單生產者-單消費者（Single-Producer, Single-Consumer, SPSC）佇列是理論上最理想的資料結構。

#### **4.2. 基礎：std::atomic 與 C++ 記憶體模型**

C++11 標準引入了 \<atomic\> 標頭檔，為無鎖程式設計提供了標準化的基礎工具 30。

- **std::atomic**：這是一個類別範本，可以將一個普通類型（如 int、bool、指標）包裝成一個原子類型。對原子類型的操作（如讀取、寫入、交換）是不可分割的。
- **硬體支援**：原子操作的效能關鍵在於硬體是否提供原生支援。std::atomic\<T\>::is_lock_free() 函式可以在執行期檢查對類型 T 的操作是否是真正的無鎖操作，還是編譯器使用內部鎖的模擬實現 22。在所有主流的現代 CPU 架構上，對原生整數類型和指標的原子操作都是無鎖的 22。  
  std::atomic_flag 是 C++ 標準中唯一**保證**始終是無鎖的類型 33。
- **記憶體順序（Memory Ordering）**：這是無鎖程式設計中最複雜但也是最關鍵的概念。它用於指定原子操作對其他記憶體存取的排序約束，以防止編譯器和 CPU 進行有害的指令重排。
  - std::memory_order_relaxed：最寬鬆的順序。只保證操作本身的原子性，不對其他記憶體操作施加任何排序約束。
  - std::memory_order_release：用於「釋放」操作（通常是寫入）。它確保在該操作**之前**的所有讀寫操作，都不會被重排到該操作**之後**。所有在此次釋放操作之前發生的寫入，對於之後「獲取」同一個原子變數的其他執行緒都是可見的。
  - std::memory_order_acquire：用於「獲取」操作（通常是讀取）。它確保在該操作**之後**的所有讀寫操作，都不會被重排到該操作**之前**。它使得執行了「釋放」操作的其他執行緒在釋放之前的所有寫入，在本次獲取操作完成後都變為可見。
  - std::memory_order_acq_rel：同時具有獲取和釋放語義，用於讀-改-寫（Read-Modify-Write）操作。
  - std::memory_order_seq_cst：最嚴格的順序，是所有原子操作的預設值。它不僅提供獲取-釋放的保證，還在所有使用此順序的原子操作之間建立了一個單一的全域順序。雖然最安全，但效能開銷也最大。

#### **4.3. 深度剖析：一個無等待的 SPSC 環形緩衝區佇列**

單生產者-單消費者（SPSC）佇列是即時音訊中最重要的資料結構，它完美地匹配了 UI 執行緒（單一生產者）與音訊執行緒（單一消費者）之間的通訊模式。以下是其基於環形緩衝區的無鎖實現原理，綜合了多個來源的精髓 22。

**核心設計**：

- 一個固定大小的陣列作為環形緩衝區，用於儲存資料。
- 兩個原子索引：head（寫入位置）和 tail（讀取位置），均為 std::atomic\<size_t\>。
- head 索引只由生產者執行緒寫入。
- tail 索引只由消費者執行緒寫入。

無鎖的「握手」機制：  
這個設計的精妙之處在於，它避免了對 head 和 tail 進行昂貴的讀-改-寫原子操作（如 compare_exchange_strong），而是通過精心設計的記憶體順序，利用簡單的原子讀寫來實現同步。

1. **生產者 push(data) 操作**：  
   C++  
   // 偽代碼  
   size_t current_head \= head.load(std::memory_order_relaxed);  
   size_t next_head \= (current_head \+ 1) % capacity;  
   if (next_head \== tail.load(std::memory_order_acquire)) {  
    // 佇列已滿  
    return false;  
   }  
   // 將資料寫入緩衝區 (非原子操作)  
   buffer\[current_head\] \= data;  
   // 更新 head 索引，並釋放記憶體屏障  
   head.store(next_head, std::memory_order_release);  
   return true;

2. **消費者 pop(\&result) 操作**：  
   C++  
   // 偽代碼  
   size_t current_tail \= tail.load(std::memory_order_relaxed);  
   if (current_tail \== head.load(std::memory_order_acquire)) {  
    // 佇列為空  
    return false;  
   }  
   // 從緩衝區讀取資料 (非原子操作)  
   \*result \= buffer\[current_tail\];  
   // 更新 tail 索引，並釋放記憶體屏障  
   tail.store((current_tail \+ 1) % capacity, std::memory_order_release);  
   return true;

**記憶體順序分析**：

- 在 push 操作中，對 buffer 的**非原子寫入**發生在對 head 的 release 儲存**之前**。
- 在 pop 操作中，對 head 的 acquire 載入發生在對 buffer 的**非原子讀取**之前。
- head 上的 release 儲存與 acquire 載入形成了一個「同步於」（synchronizes-with）的關係 33。
- 這個同步關係保證了：一旦消費者執行緒通過 acquire 載入看到了生產者寫入的新 head 值，那麼生產者在 release 儲存之前對 buffer 的所有寫入，對於消費者來說都是可見且已完成的。
- 這對 acquire-release 語義就像一道屏障，防止了編譯器或 CPU 將資料寫入操作重排到 head 索引更新之後，從而避免了消費者讀取到尚未寫入的、陳舊的資料。這正是高效能 SPSC 佇列能夠安全傳遞非原子資料的核心機制。

JUCE 框架中的 AbstractFifo 提供了一個生產級的範例，它將索引管理邏輯（AbstractFifo 物件）與資料儲存（例如 juce::AudioBuffer）分離，使用者可以將這個 FIFO 控制器應用於任何自訂的緩衝區，實現了清晰的關注點分離 18。

#### **4.4. 超越 SPSC：MPMC 佇列與生產級函式庫**

雖然 SPSC 佇列是音訊回呼的理想選擇，但在某些複雜場景下，可能需要多個生產者（例如，多個網路執行緒）或多個消費者。多生產者-多消費者（MPMC）佇列的實現要複雜得多，因為多個執行緒可能同時嘗試修改同一個索引（head 或 tail），這必須使用更昂貴的原子讀-改-寫操作（如 compare_exchange_weak 或 fetch_add）來解決爭用，並且需要處理更複雜的 ABA 問題 28。

對於 MPMC 需求，強烈建議使用經過社群廣泛測試和驗證的第三方函式庫，而不是自行實現：

- **moodycamel::ConcurrentQueue**：一個業界聞名的高效能 MPMC 無鎖佇列。其內部設計巧妙地使用了一個 SPSC 子佇列陣列，為每個生產者分配一個令牌，從而將 MPMC 問題分解為多個 SPSC 問題，極大地減少了爭用 28。
- **boost::lockfree::queue**：來自 Boost 函式庫的 MPMC 無鎖佇列實現 12。
- 其他開源實現：GitHub 上也存在許多優秀的無鎖佇列專案，提供了不同特性和效能權衡的選擇 37。

#### **4.5. 即時安全的記憶體管理：無鎖物件池**

一個常見的無鎖佇列使用模式是傳遞指向大型物件的指標，以避免在佇列中進行昂貴的物件複製 22。然而，這引出了一個更深層次的問題：這些指標所指向的物件是從哪裡來的？

如果生產者執行緒在 push 之前呼叫 new 來建立物件，而消費者執行緒在 pop 之後呼叫 delete 來銷毀物件，那麼整個系統並非真正的即時安全。因為 delete 操作本身就在音訊執行緒的「禁忌清單」上 4。即使

new 操作發生在非即時的生產者執行緒上，頻繁地與全域堆積記憶體管理器互動也可能成為效能瓶頸和爭用點 49。

解決方案是採用**無鎖物件池（Lock-Free Object Pool）**。

1. 在應用程式啟動時，預先配置一個固定數量的物件，並將它們儲存在一個池中（例如，一個無鎖佇列或堆疊）。
2. 當生產者需要一個新物件時，它不是呼叫 new，而是從物件池中「取得」（acquire）一個預先配置好的物件。這個取得操作是一個無鎖的 pop。
3. 生產者填充好物件後，將其指標推入 SPSC 通訊佇列。
4. 消費者從通訊佇列中 pop 出指標，使用完物件後，不是呼叫 delete，而是將該物件「歸還」（release）到物件池中。這個歸還操作是一個無鎖的 push。

這個 **「無鎖 SPSC 佇列 \+ 無鎖物件池」** 的組合模式，構建了一個完整的、端到端的無鎖通訊管道。它允許在執行緒間安全、高效地傳遞複雜的、動態大小的資料，而無需在即時迴圈中進行任何記憶體配置或鎖定。這是實現複雜音訊處理演算法（如事件驅動的合成器音符、粒子系統等）的關鍵架構 29。

---

## **第三部分：系統與硬體層級最佳化**

僅僅在 C++ 程式碼層級遵循即時安全規則和使用無鎖資料結構，有時仍不足以保證在複雜的通用作業系統上實現極低的、可預測的延遲。為了達到極致的效能，開發者必須深入到作業系統和硬體層面，主動控制系統的行為，以消除由外部因素引入的非確定性。本部分將探討如何通過作業系統干預和利用硬體特性來進一步降低延遲和時脈抖動。

### **第五節：作業系統干預**

#### **5.1. 控制排程器：即時執行緒優先級**

標準 C++ 的 std::thread 並未提供設定執行緒優先級的可攜式方法，因為這項功能與作業系統的排程策略緊密相關 55。因此，必須使用平台原生的 API 來進行設定。

std::thread::native_handle() 方法提供了一座橋樑，它能返回底層作業系統的執行緒控制代碼（例如在 Linux 和 macOS 上是 pthread_t），從而允許我們使用原生 API 進行進階控制 55。

- **Linux 平台**：在 Linux 上，可以使用 pthread_setschedparam() 函式來設定執行緒的排程策略（Scheduling Policy）和優先級（Priority）。對於即時音訊，最常用的策略是 SCHED_FIFO 57。
  - **SCHED_FIFO**：這是一種「先進先出」的即時排程策略。一旦一個 SCHED_FIFO 執行緒開始運行，它將持續佔用 CPU，直到它主動阻塞（例如等待 I/O）、主動讓出（sched_yield），或者被一個**更高優先級**的執行緒搶佔。它沒有時間片的概念，這使得其行為高度可預測 57。
  - **優先級設定**：Linux 的即時優先級範圍是 1 到 99，數值越高，優先級越高。這與普通執行緒的 nice 值（-20 到 \+19，數值越低優先級越高）是不同的體系。即時執行緒的優先級總是高於任何普通執行緒。為了確保音訊執行緒能夠搶佔絕大多數系統任務，包括許多核心中斷請求（IRQ）處理常式（通常運行在優先級 50 左右），一個常見且安全的做法是將音訊執行緒的優先級設定在一個較高的數值，例如 **80**。同時，應避免設定為最高的 99，以保留給最關鍵的核心任務（如看門狗計時器）使用，防止系統死鎖 57。
- **其他平台**：Windows 和 macOS 也有類似的機制。例如，在 Windows 上可以使用 SetThreadPriority() 函式，並傳入 THREAD_PRIORITY_TIME_CRITICAL 等級來提升音訊執行緒的優先級。

將音訊執行緒的優先級提升到即時等級，並非一種「最佳化」，而是在嚴苛延遲要求下的「正確性」保障。它是抵禦作業系統上其他非關鍵行程隨機搶佔 CPU 時間、從而導致音訊執行緒錯過截止期限的第一道，也是最重要的一道防線。

#### **5.2. 馴服 CPU：CPU 親和性的關鍵作用**

CPU 親和性（CPU Affinity），也稱為核心綁定（Core Pinning），是指將一個特定的執行緒「釘」在一個或多個指定的 CPU 核心上，從而指示作業系統的排程器只在這些核心上執行該執行緒，禁止將其遷移到其他核心 10。

**為何 CPU 親和性至關重要？**

1. **減少時脈抖動（Jitter）**：在預設情況下，作業系統為了負載平衡或節能，可能會在執行緒的生命週期內將其從一個 CPU 核心遷移到另一個。這個遷移過程本身需要消耗 CPU 週期，並且其發生時間是不可預測的，從而引入了延遲的抖動 59。
2. **提升快取效能**：CPU 的 L1 和 L2 快取是與特定核心綁定的。當一個執行緒長時間在同一個核心上運行時，其頻繁存取的資料和指令會被載入到該核心的快取中，極大地提升了後續存取速度。如果執行緒被遷移到一個新的核心，新核心的快取是「冷的」（Cold Cache），不包含該執行緒的資料。這將導致一連串的快取失效（Cache Miss），CPU 不得不從更慢的 L3 快取或主記憶體中重新載入資料，造成顯著的效能下降和延遲尖峰 10。

如何設定與最佳實踐：  
在 Linux 上，可以使用 pthread_setaffinity_np() 函式來設定執行緒的 CPU 親和性 10。選擇綁定到哪個核心並非隨意，遵循以下最佳實踐至關重要 59：

- **絕對不要使用核心 0**：核心 0 通常是作業系統的「引導核心」，承擔了大部分的核心行程和中斷處理。將即時執行緒綁定到這個最「繁忙」的核心上，無疑是自找麻煩，會引入大量的抖動。
- **隔離核心**：為了給即時執行緒創造一個「寧靜」的運行環境，可以通過設定核心啟動參數（如 isolcpus、nohz_full、irqaffinity）來實現核心隔離。isolcpus 會告訴排程器不要將任何普通行程調度到被隔離的核心上；nohz_full 可以減少在這些核心上的計時器中斷；irqaffinity 則可以將硬體中斷的處理轉移到其他非隔離的核心上。這樣，被隔離的核心就成為了即時應用的專屬領地。
- **了解 CPU 拓撲**：現代 CPU 的架構非常複雜，包含多個核心、多個通訊端（Socket）以及超執行緒（Hyper-Threading）和非均勻記憶體存取（NUMA）等特性。為了達到最佳效能，應該將音訊執行緒綁定到一個物理核心上，並確保其使用的記憶體是從該核心本地的 NUMA 節點上配置的，以避免昂貴的跨節點記憶體存取 10。同時，應避免將兩個 CPU 密集型的即時執行緒綁定到同一個物理核心的兩個超執行緒上，因為它們會爭搶該核心的執行單元。

### **第六節：使用 SIMD 實現指令級平行**

當音訊處理演算法本身計算密集時（例如，大量的濾波、合成或卷積運算），僅靠系統層級的最佳化可能不足以在截止期限內完成任務。此時，需要利用 CPU 的指令級平行（Instruction-Level Parallelism）能力，其中最有效的就是 SIMD（Single Instruction, Multiple Data）。

#### **6.1. SIMD 原理**

SIMD 的核心思想是，使用一條指令同時對多個資料元素執行相同的操作 62。現代 CPU 內建了寬大的向量暫存器（Vector Registers），可以一次性載入多個資料點（例如，多個浮點數樣本），然後通過專門的向量指令（Vector Instructions）進行平行處理。

- **指令集**：主流的 SIMD 指令集包括：

  - **SSE (Streaming SIMD Extensions)**：128 位元暫存器，可同時處理 4 個 32 位元單精度浮點數。
  - **AVX (Advanced Vector Extensions)**：256 位元暫存器，可同時處理 8 個 32 位元單精度浮點數。
  - AVX-512：512 位元暫存器，可同時處理 16 個 32 位元單精度浮點數。

    65

- **內建函式（Intrinsics）**：在 C++ 中，我們通過編譯器提供的「內建函式」來使用 SIMD。這些函式看起來像普通的 C++ 函式，但會被編譯器直接轉換為單一的 CPU 向量指令，例如 \_mm256_add_ps 會被編譯為 AVX 的 vaddps 指令 62。

#### **6.2. SIMD 在音訊處理中的應用：一個實踐範例**

假設我們需要將兩個立體聲音訊流（A 和 B）進行混音，並對每個流應用獨立的增益（gainA 和 gainB）。

**任務**：對緩衝區中的每個樣本 i，計算 output\[i\] \= (streamA\[i\] \* gainA) \+ (streamB\[i\] \* gainB)。

- **純量實現（Scalar Implementation）**：  
  C++  
  for (int i \= 0; i \< num_samples; \++i) {  
   output\[i\] \= (streamA\[i\] \* gainA) \+ (streamB\[i\] \* gainB);  
  }

  這個迴圈對每個樣本執行 2 次乘法和 1 次加法。處理 8 個樣本需要 24 次浮點運算。

- **SIMD (AVX) 實現**：  
  C++  
  // 偽代碼  
  // 將增益廣播到向量暫存器的所有元素  
  \_\_m256 v_gainA \= \_mm256_set1_ps(gainA);  
  \_\_m256 v_gainB \= \_mm256_set1_ps(gainB);

  for (int i \= 0; i \< num_samples; i \+= 8) {  
   // 1\. 從記憶體載入 8 個樣本到 AVX 暫存器  
   \_\_m256 v_streamA \= \_mm256_loadu_ps(\&streamA\[i\]);  
   \_\_m256 v_streamB \= \_mm256_loadu_ps(\&streamB\[i\]);

      // 2\. 平行乘法
      \_\_m256 v\_resultA \= \_mm256\_mul\_ps(v\_streamA, v\_gainA);
      \_\_m256 v\_resultB \= \_mm256\_mul\_ps(v\_streamB, v\_gainB);

      // 3\. 平行加法
      \_\_m256 v\_output \= \_mm256\_add\_ps(v\_resultA, v\_resultB);

      // 4\. 將 8 個結果樣本存回記憶體
      \_mm256\_storeu\_ps(\&output\[i\], v\_output);

  }

  在這個版本中，我們用大約 5-6 條向量指令就處理了 8 個樣本，而不是 24 條純量指令。這極大地提升了計算吞吐量，使得在同樣的時間內可以完成更多的處理工作 65。

#### **6.3. 資料佈局的重要性：SoA vs. AoS**

SIMD 的效能極度依賴於資料在記憶體中的佈局方式。為了能夠用一條指令載入多個資料元素到向量暫存器，這些資料元素必須在記憶體中是連續的。

- **結構陣列（Array of Structures, AoS）**：這是物件導向程式設計中常見的佈局，例如 struct StereoSample { float left; float right; }; std::vector\<StereoSample\> buffer;。這種 LRLRLR... 的交錯（interleaved）佈局對 SIMD 非常不友好。要載入 8 個左聲道樣本，CPU 需要進行 8 次分散的記憶體讀取，無法發揮 SIMD 的優勢。
- **陣列結構（Structure of Arrays, SoA）**：這種佈局將不同通道的資料分開存放，例如 struct StereoBuffer { std::vector\<float\> left_channel; std::vector\<float\> right_channel; };。這種 LLLL...RRRR... 的平面（planar）佈局對 SIMD 極為友好。所有左聲道樣本在記憶體中是連續的，可以用一條 \_mm256_loadu_ps 指令一次性載入 8 個樣本 67。

因此，在設計需要 SIMD 最佳化的音訊處理管線時，應優先考慮使用 SoA（平面）資料佈局。

#### **6.4. 導航陷阱**

使用 SIMD 雖然強大，但也伴隨著一些需要小心處理的陷阱：

- **AVX/SSE 過渡懲罰**：在同一個函式或執行緒中混合使用舊的、非 VEX 編碼的 SSE 指令與新的、VEX 編碼的 AVX 指令，可能會導致嚴重的效能懲罰。當 CPU 從執行 AVX 指令切換到執行舊版 SSE 指令時，它需要儲存 YMM 暫存器的高 128 位元；反之切換回來時則需要恢復。這個儲存和恢復的過程會帶來數十個週期的延遲。避免這個問題的方法有：
  1. 使用編譯器旗標（如 GCC/Clang 的 \-mavx 或 MSVC 的 /arch:AVX）來指示編譯器將所有 SSE 指令也用 VEX 進行編碼，從而消除過渡。
  2. 在從 AVX 程式碼區塊返回到可能執行 SSE 程式碼的區塊之前，手動呼叫 \_mm256_zeroupper() 內建函式。這個指令會將所有 YMM 暫存器的高 128 位元清零，告訴硬體無需進行儲存和恢復操作 69。
- **資料對齊**：SIMD 的載入/儲存指令在處理的記憶體位址是向量寬度的整數倍時（例如，對於 AVX 是 32 位元組對齊）效能最高。雖然有提供非對齊（unaligned）版本的指令（如 \_mm256_loadu_ps），但它們在某些 CPU 架構上可能會稍慢，或者被分解為多個微操作。為了極致效能，應盡可能確保資料緩衝區的起始位址是對齊的 63。

---

## **第四部分：高階架構與案例研究**

在前幾部分建立了低延遲音訊程式設計的基礎、並行處理和系統層級最佳化的知識後，本部分將這些技術綜合起來，探討更複雜的、用於解決實際問題的高階架構模式，並通過分析一個真實世界的開源專案來展示這些技術的應用。

### **第七節：複雜處理的架構模式**

#### **7.1. 卸載重度計算**

**問題**：當一個音訊處理任務（例如，一個長卷積殘響、一個高解析度的 FFT 分析，或是一個複雜的物理模型合成）其單次計算時間本身就超過了一個音訊緩衝區的處理截止期限時，我們該如何處理？顯然，這樣的任務不能直接在即時音訊回呼中同步執行。

**架構解決方案**：採用一個專門的「工作執行緒」（Worker Thread）來非同步地執行這些重度計算任務，並通過無鎖佇列與即時音訊執行緒進行通訊。

一個典型的「計算卸載」架構如下：

1. **主/UI 執行緒（生產者）**：當需要觸發一次重度計算時（例如，使用者載入了一個新的殘響脈衝響應），主執行緒會建立一個「工作項目」（Job）。這個工作項目通常是一個結構，包含了計算所需的全部參數、指向輸入資料的指標，以及一個用於存放結果的空間。然後，它將這個工作項目（或指向它的指標）推入一個無鎖的、多生產者-單消費者（MPSC）佇列中，供工作執行緒使用。
2. **工作執行緒（處理者）**：這是一個或多個獨立的、長時間運行的執行緒。它們的主迴圈就是不斷地嘗試從 MPSC 佇列中取出工作項目。一旦取得一個工作項目，它就開始執行耗時的計算。這個過程完全獨立於即時音訊執行緒，不會對其造成任何阻塞。
3. **完成信號與資料交換**：當工作執行緒完成計算後，它需要一種方式來通知音訊執行緒「新的結果已經準備好了」。一個高效、輕量且即時安全的方法是使用 std::atomic_flag。工作執行緒可以將計算結果寫入一個共享的、預先配置好的記憶體位置，然後通過呼叫 test_and_set() 來設定一個 std::atomic_flag，表示資料已就緒 22。
4. **音訊執行緒（消費者）**：在每個音訊回呼中，音訊執行緒會以非阻塞的方式檢查這個 std::atomic_flag。如果旗標被設定，它就知道有新的處理結果可用。此時，它可以安全地讀取新的結果資料（例如，通過一個原子指標交換來切換到新的資料緩衝區），並在讀取完成後，呼叫 clear() 來重設旗標，告知工作執行緒它可以準備下一次計算了。為了平滑地過渡到新資料，通常會採用交叉淡化（Crossfade）等技術。

這種架構模式巧妙地結合了生產者-消費者模型、無鎖佇列和輕量級的原子信號機制，將非即時的重度計算與即時的音訊播放完全解耦，是處理複雜音訊效果的標準方法之一。

#### **7.2. 模組化合成與效果：AudioProcessorGraph 模型**

許多音訊應用，如數位音訊工作站（DAW）、虛擬混音台或吉他效果器鏈，都需要一個能夠動態地將音訊訊號路由到一系列處理器中的系統。JUCE 框架中的 juce::AudioProcessorGraph 是實現此類架構的一個絕佳案例研究 75。

**核心概念**：

- **節點（Nodes）**：圖中的每個節點都是一個 juce::AudioProcessor 的實例。這可以是一個濾波器、一個合成器、一個增益控制器，甚至是另一個巢狀的 AudioProcessorGraph 76。
- **連接（Connections）**：圖中的有向邊代表了節點之間輸入/輸出通道的連接，定義了音訊訊號的流動路徑。例如，可以將一個振盪器節點的輸出連接到一個濾波器節點的輸入 76。
- **I/O 處理器**：AudioProcessorGraph 提供了特殊的節點類型（AudioGraphIOProcessor），用於代表整個圖的總音訊和 MIDI 輸入/輸出，充當訊號進入和離開圖的埠 76。

處理順序問題：  
AudioProcessorGraph 本質上是一個有向無環圖（Directed Acyclic Graph, DAG）。為了正確地處理音訊，必須以一種尊重依賴關係的順序來執行圖中的節點。如果節點 A 的輸出連接到節點 B 的輸入，那麼節點 A 必須在節點 B 之前被處理。  
這引出了一個根本性的問題：給定一個複雜的、由節點和連接構成的圖，系統如何自動確定正確的、線性的處理序列？這個問題的答案揭示了 AudioProcessorGraph 內部運作的核心演算法。

這個問題在圖論中被稱為**拓撲排序（Topological Sort）**。拓撲排序的定義是：為一個有向無環圖的所有頂點提供一個線性排序，使得對於圖中每一條從頂點 u 到頂點 v 的有向邊，u 在排序中都出現在 v 之前 80。

因此，AudioProcessorGraph 的內部處理邏輯必然依賴於拓撲排序。每當圖的結構發生變化時（例如，使用者新增或刪除了一個效果器，或改變了連接），AudioProcessorGraph 就會對其所有節點執行一次拓撲排序。這個排序演算法會產生一個線性的、已排序的節點列表。之後，在每個音訊回呼中，processBlock 函式只需簡單地按照這個預先計算好的順序，依次呼叫列表中每個節點的 processBlock 方法即可。

這也完美地解釋了為什麼在 AudioProcessorGraph 中不允許存在反饋迴路（Feedback Loops），即圖中不能有環（Cycles）。一個包含環的圖是無法進行拓撲排序的 80。為了防止使用者建立無效的圖，JUCE 提供了一個輔助函式

isAnInputTo()，它會在新增連接之前進行一次遞迴的深度優先搜尋，檢查新的連接是否會導致目標節點成為源節點的上游，從而有效地檢測並阻止環的形成 77。這是一個將經典電腦科學演算法應用於解決複雜音訊訊號路由問題的典範。

### **第八節：案例研究：開源合成器 Surge XT**

為了將前述的理論和技術與現實世界連結，本節將簡要分析廣受歡迎的開源合成器專案 Surge XT。這個專案是展示現代 C++ 低延遲音訊軟體開發實踐的一個絕佳範例。

Surge XT 是一個功能強大且複雜的混合式合成器，它最初是一個商業產品，後來開源，並由一個活躍的社群持續開發 83。

**架構與低延遲實踐**：

- **建構系統與平台支援**：該專案使用現代化的 CMake 作為其建構系統，支援跨平台編譯（Windows, macOS, Linux）和多種外掛程式格式（VST3, AU, CLAP），展示了良好的工程實踐 83。
- **對 SIMD 的依賴**：Surge XT 的系統要求明確指出需要支援 SSE2 指令集的 CPU，這直接表明其核心音訊引擎廣泛利用了 SIMD 技術來進行效能最佳化，以處理其複雜的振盪器和濾波器演算法 83。
- **低延遲驅動支援**：在 Windows 平台上，專案的建構選項中包含了對 ASIO 的支援。ASIO 是 Windows 上實現低延遲音訊的首選驅動程式模型，這顯示了開發團隊對低延遲效能的重視 84。
- **模組化設計**：Surge XT 的程式碼庫被組織成多個獨立的子專案和函式庫，例如 sst-cpputils（通用工具）、sst-basic-blocks（基礎音訊處理模組）等 85。這種模組化的設計不僅有利於程式碼的維護和重用，也反映了一種將複雜系統分解為可管理、可測試的元件的成熟架構思想。
- **可重用元件的開發**：專案團隊將其核心的音高微調（Microtuning）功能提取出來，製作成一個獨立的、僅含標頭檔的、採用 MIT 授權的 C++ 函式庫 Tunings.h 87。這表明團隊致力於建立高效能、可重用的元件，並鼓勵更廣泛的社群應用。

通過對 Surge XT 的簡要分析，可以看出，本報告中討論的各種技術——從 SIMD 最佳化、對低延遲驅動的支援，到模組化的軟體架構——並非僅僅是理論上的概念，而是被活躍地應用於開發尖端的、高效能的現代音訊軟體之中。

---

## **第五部分：綜合與最佳實踐**

經過對低延遲音訊處理中各個層面的技術進行深入探討後，本部分旨在將這些知識綜合起來，為開發者提供一個清晰的決策框架和一份簡潔的技術摘要，以指導在實際專案中的架構選擇和技術應用。

### **第九節：低延遲設計的決策框架**

低延遲音訊設計是一個整體性的工程，不存在一體適用的「最佳」解決方案。相反，最佳的架構選擇取決於專案的具體延遲目標。延遲目標越低，所要求的架構複雜度和技術深度就越高。以下提供一個決策流程圖，幫助開發者根據其延遲目標來選擇合適的技術組合。

**決策流程：**

1. **定義延遲目標 (End-to-End Latency Target)**
   - **\> 20 毫秒 (例如，非互動式播放器、離線處理)**
     - **緩衝策略**：使用較大的緩衝區和簡單的雙重緩衝機制即可滿足穩定性要求。
     - **執行緒通訊**：可以使用基於鎖的佇列（std::mutex \+ std::condition_variable），因為較長的處理截止期限可以容忍偶爾的鎖爭用和阻塞。但仍需注意避免在即時執行緒中執行長時間持有鎖的操作。
     - **系統最佳化**：通常無需進行執行緒優先級或 CPU 親和性的設定。
   - **10-20 毫秒 (例如，入門級即時效果器、半專業應用)**
     - **緩衝策略**：應考慮使用三重緩衝或 N-緩衝策略，以增加對處理尖峰的容忍度 5。
     - **執行緒通訊**：**必須**使用無鎖佇列（如 SPSC 佇列）來進行主執行緒與音訊執行緒之間的通訊，以完全避免優先級反轉的風險 8。
     - **系統最佳化**：建議將音訊執行緒的優先級提升到即時等級，但可能還不需要嚴格的 CPU 核心隔離。
   - **\< 10 毫秒 (例如，專業級虛擬樂器、現場監聽、高階 DAW)**
     - **緩衝策略**：必須使用較小的緩衝區，並結合 N-緩衝策略來最大化穩定性 5。
     - **執行緒通訊**：無鎖 SPSC 佇列是基本要求。對於複雜物件的傳遞，應結合使用無鎖物件池 49。
     - **系統最佳化**：將音訊執行緒設定為高即時優先級（SCHED_FIFO）和設定 CPU 親和性（綁定到隔離的核心）是**強制性**的，而非可選。這是最小化時脈抖動和確保可預測性的關鍵 57。
2. **評估計算複雜度**
   - **演算法是否為計算密集型？**
     - **是**：則必須使用 SIMD 進行向量化最佳化，以在單個緩衝區的處理時間內完成計算。同時，應採用對 SIMD 友好的 SoA 資料佈局 65。
     - **否**：則可以優先關注記憶體存取模式和演算法的整體效率。
   - **單個處理任務的執行時間是否超過緩衝區時長？**
     - **是**：則必須採用「計算卸載」架構，將該任務移至獨立的工作執行緒中非同步執行，並通過無鎖佇列和原子旗標與音訊執行緒同步結果 22。
     - **否**：則可以在音訊回呼中同步執行。
3. **通用規則**
   - 無論延遲目標為何，**「禁忌清單」** 中的所有規則都嚴格適用於即時音訊執行緒 4。這是保證任何即時音訊系統穩定運行的基礎。

### **第十節：技術與權衡總結表**

為了提供一個快速參考，下表總結了本報告中討論的核心技術，並對其主要目標、對延遲和 CPU 的影響、實現複雜度以及關鍵的權衡進行了比較。這個矩陣可以作為架構師和開發者在進行高層次設計決策時的速查指南。

| 技術 (Technique)                       | 主要目標 (Primary Goal)   | 延遲影響 (Latency Impact)          | CPU 開銷 (CPU Overhead)  | 實現複雜度 (Implementation Complexity) | 關鍵權衡 (Key Trade-off)                       |
| :------------------------------------- | :------------------------ | :--------------------------------- | :----------------------- | :------------------------------------- | :--------------------------------------------- |
| 雙重緩衝 (Double Buffering)            | 防止串流時的音訊撕裂/瑕疵 | 基準線 (例如，2 倍緩衝區大小)      | 極小                     | 低                                     | 對於超過單個緩衝區時長的處理尖峰較為脆弱。     |
| 三重/N-緩衝 (Triple/N-Buffering)       | 吸收 CPU 尖峰和處理抖動   | 更高 (例如，3 倍或 N 倍緩衝區大小) | 低至中                   | 低                                     | 以增加端到端延遲為代價換取更高的穩定性。       |
| 環形緩衝區 (Ring Buffer)               | 實現高效的串流資料流      | 不適用 (資料結構)                  | 極小                     | 低至中                                 | 佇列的基礎；需要謹慎的索引管理。               |
| 基於鎖的佇列 (Lock-Based Queue)        | 簡單的執行緒安全通訊      | **不可預測** (可能非常高)          | 低 (空閒時), 高 (爭用時) | 中                                     | **優先級反轉**。對即時音訊執行緒不安全。       |
| 無鎖 SPSC 佇列 (Lock-Free SPSC Queue)  | 無等待、確定性的通訊      | **極小且可預測**                   | 低                       | 高                                     | 正確實現極為複雜 (需精通記憶體順序)。          |
| CPU 親和性 (CPU Affinity)              | 減少因內容切換引起的抖動  | 降低延遲的**變異數**/抖動          | 極小                     | 中 (平台特定)                          | 獨佔一個 CPU 核心；降低了系統整體的平行能力。  |
| 即時優先級 (Real-Time Priority)        | 防止被低優先級任務搶佔    | 降低延遲的**變異數**/抖動          | 極小                     | 中 (平台特定)                          | 若使用不當，可能導致其他行程「餓死」。         |
| SIMD 向量化 (SIMD Vectorization)       | 提升計算吞吐量            | 減少單個緩衝區內的處理時間         | 低 (可降低總 CPU 使用率) | 高                                     | 程式碼複雜度高，有資料佈局和平台特定性的限制。 |
| 工作執行緒卸載 (Worker Thread Offload) | 處理長於截止期限的任務    | 增加該特定任務的延遲               | 高 (使用額外執行緒)      | 高                                     | 為非同步獲取結果而引入的架構複雜性。           |

#### **Works cited**

1. Implementing low-latency shared/exclusive mode audio output/duplex | nyanpasu64's blog, accessed July 8, 2025, [https://nyanpasu64.gitlab.io/blog/low-latency-audio-output-duplex-alsa/](https://nyanpasu64.gitlab.io/blog/low-latency-audio-output-duplex-alsa/)
2. Audio I/O: Buffering, Latency, and Throughput \- MATLAB & Simulink \- MathWorks, accessed July 8, 2025, [https://la.mathworks.com/help/audio/gs/audio-io-buffering-latency-and-throughput.html](https://la.mathworks.com/help/audio/gs/audio-io-buffering-latency-and-throughput.html)
3. A Comprehensive Guide to Analog-to-Digital Converters (ADCs) | Article | MPS, accessed July 8, 2025, [https://www.monolithicpower.com/en/learning/resources/comprehensive-guide-to-analog-to-digital-converters](https://www.monolithicpower.com/en/learning/resources/comprehensive-guide-to-analog-to-digital-converters)
4. Real-time audio programming 101: time waits for nothing \- Ross Bencina, accessed July 8, 2025, [http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing)
5. waveOut API latency \- C++ Forum, accessed July 8, 2025, [https://cplusplus.com/forum/general/121929/](https://cplusplus.com/forum/general/121929/)
6. Pre-loading audio buffers \- what is reasonable and reliable? \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/29137526/pre-loading-audio-buffers-what-is-reasonable-and-reliable](https://stackoverflow.com/questions/29137526/pre-loading-audio-buffers-what-is-reasonable-and-reliable)
7. \[Tutorial\] Audio Programming into with C++ (pt. 1\) : r/cpp \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/cpp/comments/b1888z/tutorial_audio_programming_into_with_c_pt_1/](https://www.reddit.com/r/cpp/comments/b1888z/tutorial_audio_programming_into_with_c_pt_1/)
8. A Fast Lock-Free Queue for C++ \- moodycamel.com, accessed July 8, 2025, [https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++](https://moodycamel.com/blog/2013/a-fast-lock-free-queue-for-c++)
9. Real-time confessions: the most common “... \- Audio Developer Conference, accessed July 8, 2025, [https://adc23.sched.com/event/1PueK/real-time-confessions-the-most-common-sins-in-real-time-code](https://adc23.sched.com/event/1PueK/real-time-confessions-the-most-common-sins-in-real-time-code)
10. Writing Low-Latency C++ Applications \- Medium, accessed July 8, 2025, [https://medium.com/@AlexanderObregon/writing-low-latency-c-applications-f759c94f52f8](https://medium.com/@AlexanderObregon/writing-low-latency-c-applications-f759c94f52f8)
11. Forbidden C++ : r/cpp \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/cpp/comments/jks8f3/forbidden_c/](https://www.reddit.com/r/cpp/comments/jks8f3/forbidden_c/)
12. compare among boost::lockfree::spsc_queue, boost::lockfree::queue, serial queue operation \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/29662040/compare-among-boostlockfreespsc-queue-boostlockfreequeue-serial-queue](https://stackoverflow.com/questions/29662040/compare-among-boostlockfreespsc-queue-boostlockfreequeue-serial-queue)
13. Taming Real-Time Logging: Lessons Learned From the Trenches ..., accessed July 8, 2025, [https://www.youtube.com/watch?v=4KFFMGTQIFM](https://www.youtube.com/watch?v=4KFFMGTQIFM)
14. how to implement ring/circular/double buffers for external flash ..., accessed July 8, 2025, [https://e2e.ti.com/support/microcontrollers/msp-low-power-microcontrollers-group/msp430/f/msp-low-power-microcontroller-forum/128218/how-to-implement-ring-circular-double-buffers-for-external-flash-applications](https://e2e.ti.com/support/microcontrollers/msp-low-power-microcontrollers-group/msp430/f/msp-low-power-microcontroller-forum/128218/how-to-implement-ring-circular-double-buffers-for-external-flash-applications)
15. Trying to understand the general scheme of how real-time audio signal processing works on embedded system, accessed July 8, 2025, [https://dsp.stackexchange.com/questions/91558/trying-to-understand-the-general-scheme-of-how-real-time-audio-signal-processing](https://dsp.stackexchange.com/questions/91558/trying-to-understand-the-general-scheme-of-how-real-time-audio-signal-processing)
16. Double or Triple Buffered VSYNC for Lowest Input Lag : r/pcgaming \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/pcgaming/comments/7eiaq4/double_or_triple_buffered_vsync_for_lowest_input/](https://www.reddit.com/r/pcgaming/comments/7eiaq4/double_or_triple_buffered_vsync_for_lowest_input/)
17. Audio Settings Help with Buffer switch : r/FL_Studio \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/FL_Studio/comments/n2gsrb/audio_settings_help_with_buffer_switch/](https://www.reddit.com/r/FL_Studio/comments/n2gsrb/audio_settings_help_with_buffer_switch/)
18. AbstractFifo Class Reference \- JUCE: Tags, accessed July 8, 2025, [https://docs.juce.com/master/classAbstractFifo.html](https://docs.juce.com/master/classAbstractFifo.html)
19. JUCE/modules/juce_core/containers/juce_AbstractFifo.cpp at ..., accessed July 8, 2025, [https://github.com/juce-framework/JUCE/blob/master/modules/juce_core/containers/juce_AbstractFifo.cpp](https://github.com/juce-framework/JUCE/blob/master/modules/juce_core/containers/juce_AbstractFifo.cpp)
20. Concurrency, meters, JUCE classes \- Audio Plugins, accessed July 8, 2025, [https://forum.juce.com/t/concurrency-meters-juce-classes/18104](https://forum.juce.com/t/concurrency-meters-juce-classes/18104)
21. Need explaination regarding multithreading and double/triple buffering : r/gamedev \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/gamedev/comments/dfisv7/need_explaination_regarding_multithreading_and/](https://www.reddit.com/r/gamedev/comments/dfisv7/need_explaination_regarding_multithreading_and/)
22. Wait-Free Programming From Scratch | by Jatin Chowdhury \- Medium, accessed July 8, 2025, [https://jatinchowdhury18.medium.com/wait-free-programming-from-scratch-5ac6a65c23c4](https://jatinchowdhury18.medium.com/wait-free-programming-from-scratch-5ac6a65c23c4)
23. Multithreading with C++ —Producer-Consumer Problem | Nerd For Tech \- Medium, accessed July 8, 2025, [https://medium.com/nerd-for-tech/hands-on-multithreading-with-c-04-producer-consumer-problem-26abdddc485d](https://medium.com/nerd-for-tech/hands-on-multithreading-with-c-04-producer-consumer-problem-26abdddc485d)
24. Producer-Consumer model \- c++ \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/61478260/producer-consumer-model](https://stackoverflow.com/questions/61478260/producer-consumer-model)
25. Implementing the Real-World Producer-Consumer Problem | CodeSignal Learn, accessed July 8, 2025, [https://codesignal.com/learn/courses/practice-tasks-for-concurrency-essentials/lessons/implementing-the-real-world-producer-consumer-problem](https://codesignal.com/learn/courses/practice-tasks-for-concurrency-essentials/lessons/implementing-the-real-world-producer-consumer-problem)
26. Producer-Consumer Problem and its Implementation with C++ \- Tutorialspoint, accessed July 8, 2025, [https://www.tutorialspoint.com/producer-consumer-problem-and-its-implementation-with-cplusplus](https://www.tutorialspoint.com/producer-consumer-problem-and-its-implementation-with-cplusplus)
27. Consumer/Producer pattern in C++11 : r/cpp \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/cpp/comments/1kkca5/consumerproducer_pattern_in_c11/](https://www.reddit.com/r/cpp/comments/1kkca5/consumerproducer_pattern_in_c11/)
28. What's the state of lock-free queue implementations in Ada? (for audio programming), accessed July 8, 2025, [https://www.reddit.com/r/ada/comments/1jfbcod/whats_the_state_of_lockfree_queue_implementations/](https://www.reddit.com/r/ada/comments/1jfbcod/whats_the_state_of_lockfree_queue_implementations/)
29. Chapter 19\. Boost.Lockfree, accessed July 8, 2025, [https://www.boost.org/libs/lockfree/](https://www.boost.org/libs/lockfree/)
30. C++ Atomic::operator--() function. \- Tutorialspoint, accessed July 8, 2025, [https://www.tutorialspoint.com/cpp_standard_library/cpp_atomic_operator_sub.htm](https://www.tutorialspoint.com/cpp_standard_library/cpp_atomic_operator_sub.htm)
31. A simple guide to atomics in C++ \- Dev Genius, accessed July 8, 2025, [https://blog.devgenius.io/a-simple-guide-to-atomics-in-c-670fc4842c8b](https://blog.devgenius.io/a-simple-guide-to-atomics-in-c-670fc4842c8b)
32. Atomics And Concurrency \- Medium, accessed July 8, 2025, [https://medium.com/@redixhumayun/atomics-and-concurrency-70b57ce50d27](https://medium.com/@redixhumayun/atomics-and-concurrency-70b57ce50d27)
33. Lock-Free Single-Producer \- Single Consumer Circular Queue ..., accessed July 8, 2025, [https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular](https://www.codeproject.com/Articles/43510/Lock-Free-Single-Producer-Single-Consumer-Circular)
34. What lock-free primitives do people actually use to do lock-free audio processing in c++?, accessed July 8, 2025, [https://stackoverflow.com/questions/64345949/what-lock-free-primitives-do-people-actually-use-to-do-lock-free-audio-processin](https://stackoverflow.com/questions/64345949/what-lock-free-primitives-do-people-actually-use-to-do-lock-free-audio-processin)
35. The Atomic Flag – MC++ BLOG \- Modernes C++, accessed July 8, 2025, [https://www.modernescpp.com/index.php/the-atomic-flag/](https://www.modernescpp.com/index.php/the-atomic-flag/)
36. SPSC Overwriting Queue \- Google Groups, accessed July 8, 2025, [https://groups.google.com/g/lock-free/c/P1rkoA0Oy7Y](https://groups.google.com/g/lock-free/c/P1rkoA0Oy7Y)
37. GitHub \- alpc62/lock-free-queue: C/C++Non-Blocking Lock-Free ..., accessed July 8, 2025, [https://github.com/alpc62/lock-free-queue](https://github.com/alpc62/lock-free-queue)
38. std::memory_order \- cppreference.com \- C++ reference, accessed July 8, 2025, [https://en.cppreference.com/w/cpp/atomic/memory_order.html](https://en.cppreference.com/w/cpp/atomic/memory_order.html)
39. Use of std::memory_order_consume in the Folly's lock free SPSC queue \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/36159251/use-of-stdmemory-order-consume-in-the-follys-lock-free-spsc-queue](https://stackoverflow.com/questions/36159251/use-of-stdmemory-order-consume-in-the-follys-lock-free-spsc-queue)
40. A Portable Lock-free Bounded Queue \- Reed Milewicz, accessed July 8, 2025, [https://rmmilewi.github.io/files/lockfreequeue16.pdf](https://rmmilewi.github.io/files/lockfreequeue16.pdf)
41. Building a fast SPSC queue: atomics, memory ordering, false sharing : r/cpp \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/cpp/comments/1ivlv7e/building_a_fast_spsc_queue_atomics_memory/](https://www.reddit.com/r/cpp/comments/1ivlv7e/building_a_fast_spsc_queue_atomics_memory/)
42. I need a single-consumer, multi-ad-hoc-producer queue : r/cpp \- Reddit, accessed July 8, 2025, [https://www.reddit.com/r/cpp/comments/1f7xycv/i_need_a_singleconsumer_multiadhocproducer_queue/](https://www.reddit.com/r/cpp/comments/1f7xycv/i_need_a_singleconsumer_multiadhocproducer_queue/)
43. Producer-Consumer Queues \- 1024cores, accessed July 8, 2025, [https://www.1024cores.net/home/lock-free-algorithms/queues](https://www.1024cores.net/home/lock-free-algorithms/queues)
44. cameron314/concurrentqueue: A fast multi-producer, multi-consumer lock-free concurrent queue for C++11 \- GitHub, accessed July 8, 2025, [https://github.com/cameron314/concurrentqueue](https://github.com/cameron314/concurrentqueue)
45. max0x7ba/atomic_queue: C++14 lock-free queue. \- GitHub, accessed July 8, 2025, [https://github.com/max0x7ba/atomic_queue](https://github.com/max0x7ba/atomic_queue)
46. phaag/lfq: Lock-Free FIFO Queue \- GitHub, accessed July 8, 2025, [https://github.com/phaag/lfq](https://github.com/phaag/lfq)
47. Taymindis/lfqueue: lock-free FIFO queue by C native built it, easy built cross platform(no extra dependencies needed) , guarantee thread safety memory management ever\! \- GitHub, accessed July 8, 2025, [https://github.com/Taymindis/lfqueue](https://github.com/Taymindis/lfqueue)
48. craflin/LockFreeQueue: Lock-free multi-producer multi-consumer ring buffer FIFO queue., accessed July 8, 2025, [https://github.com/craflin/LockFreeQueue](https://github.com/craflin/LockFreeQueue)
49. Memory management in a lock free queue \- c++ \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/6461236/memory-management-in-a-lock-free-queue](https://stackoverflow.com/questions/6461236/memory-management-in-a-lock-free-queue)
50. C++ implementation of an object pool pattern \- GitHub, accessed July 8, 2025, [https://github.com/massimo-marino/object-pool](https://github.com/massimo-marino/object-pool)
51. A simple template implementation of an object pool in C++. \- GitHub, accessed July 8, 2025, [https://github.com/Kautenja/object-pool](https://github.com/Kautenja/object-pool)
52. C++ Object Pool, Thread-Safe \- GitHub Gist, accessed July 8, 2025, [https://gist.github.com/tyan-boot/552c39580a5e6fa40c8c859877a37671](https://gist.github.com/tyan-boot/552c39580a5e6fa40c8c859877a37671)
53. benpm/cppmempool: very simple C++ heterogeneous memory pool with thread safety, accessed July 8, 2025, [https://github.com/benpm/cppmempool](https://github.com/benpm/cppmempool)
54. Lock Free Queue \-- Single Producer, Multiple Consumers \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/2702328/lock-free-queue-single-producer-multiple-consumers](https://stackoverflow.com/questions/2702328/lock-free-queue-single-producer-multiple-consumers)
55. Thread priorities \- C++ High Performance \[Book\] \- O'Reilly Media, accessed July 8, 2025, [https://www.oreilly.com/library/view/c-high-performance/9781787120952/49085824-484d-42df-bf06-fc32083175d8.xhtml](https://www.oreilly.com/library/view/c-high-performance/9781787120952/49085824-484d-42df-bf06-fc32083175d8.xhtml)
56. How to assign priority to threads in multi-thread processes in C Linux \- Quora, accessed July 8, 2025, [https://www.quora.com/How-can-I-assign-priority-to-threads-in-multi-thread-processes-in-C-Linux](https://www.quora.com/How-can-I-assign-priority-to-threads-in-multi-thread-processes-in-C-Linux)
57. Real-time programming with Linux, part 4: C++ application tutorial ..., accessed July 8, 2025, [https://shuhaowu.com/blog/2022/04-linux-rt-appdev-part4.html](https://shuhaowu.com/blog/2022/04-linux-rt-appdev-part4.html)
58. How to prioritize (or set scheduling policy for) the 'manager' and 'worker' threads of a process? \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/28008127/how-to-prioritize-or-set-scheduling-policy-for-the-manager-and-worker-thre](https://stackoverflow.com/questions/28008127/how-to-prioritize-or-set-scheduling-policy-for-the-manager-and-worker-thre)
59. On pinning and isolating CPU cores \- Manuel Bernhardt, accessed July 8, 2025, [https://manuel.bernhardt.io/posts/2023-11-16-core-pinning/](https://manuel.bernhardt.io/posts/2023-11-16-core-pinning/)
60. should I use thread affinity for "latency-critical" threads? \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/15295438/should-i-use-thread-affinity-for-latency-critical-threads](https://stackoverflow.com/questions/15295438/should-i-use-thread-affinity-for-latency-critical-threads)
61. How to set thread affinity to either performance or efficient cores? \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/77410990/how-to-set-thread-affinity-to-either-performance-or-efficient-cores](https://stackoverflow.com/questions/77410990/how-to-set-thread-affinity-to-either-performance-or-efficient-cores)
62. What is SIMD and how to use it \- Medium, accessed July 8, 2025, [https://medium.com/@anilcangulkaya7/what-is-simd-and-how-to-use-it-3d1125faac89](https://medium.com/@anilcangulkaya7/what-is-simd-and-how-to-use-it-3d1125faac89)
63. SIMD: Supercharging Your Code with Parallel Processing \- DEV Community, accessed July 8, 2025, [https://dev.to/ashokan/simd-supercharging-your-code-with-parallel-processing-2h5b](https://dev.to/ashokan/simd-supercharging-your-code-with-parallel-processing-2h5b)
64. Single instruction, multiple data \- Wikipedia, accessed July 8, 2025, [https://en.wikipedia.org/wiki/Single_instruction,\_multiple_data](https://en.wikipedia.org/wiki/Single_instruction,_multiple_data)
65. Performance differences when using AVX instructions \- Albert Herd, accessed July 8, 2025, [https://albertherd.com/2018/01/11/performance-differences-when-using-avx-instructions/](https://albertherd.com/2018/01/11/performance-differences-when-using-avx-instructions/)
66. Faster Tape Emulation with SIMD \- by Jatin Chowdhury \- Medium, accessed July 8, 2025, [https://medium.com/codex/faster-tape-emulation-with-simd-49287d7b24cf](https://medium.com/codex/faster-tape-emulation-with-simd-49287d7b24cf)
67. A practical guide to SSE SIMD with C++ \- Tuomas Tonteri, accessed July 8, 2025, [http://sci.tuomastonteri.fi/programming/sse](http://sci.tuomastonteri.fi/programming/sse)
68. How does SIMD (avx) processing work? for example, if I want 10 32 bit floats how do i fit in a 256 bit avx vector? \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/77574422/how-does-simd-avx-processing-work-for-example-if-i-want-10-32-bit-floats-how](https://stackoverflow.com/questions/77574422/how-does-simd-avx-processing-work-for-example-if-i-want-10-32-bit-floats-how)
69. SSE and AVX intrinsics mixture \- c++ \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/18319488/sse-and-avx-intrinsics-mixture](https://stackoverflow.com/questions/18319488/sse-and-avx-intrinsics-mixture)
70. Mixing SSE2 and AVX intrinsics with different compilers \- Stack Overflow, accessed July 8, 2025, [https://stackoverflow.com/questions/49226452/mixing-sse2-and-avx-intrinsics-with-different-compilers](https://stackoverflow.com/questions/49226452/mixing-sse2-and-avx-intrinsics-with-different-compilers)
71. Avoiding AVX-SSE Transition Penalties \- Intel, accessed July 8, 2025, [https://www.intel.com/content/dam/develop/external/us/en/documents/11mc12-avoiding-2bavx-sse-2btransition-2bpenalties-2brh-2bfinal-809104.pdf](https://www.intel.com/content/dam/develop/external/us/en/documents/11mc12-avoiding-2bavx-sse-2btransition-2bpenalties-2brh-2bfinal-809104.pdf)
72. SSE & AVX: x86 SIMD \- UAF CS, accessed July 8, 2025, [https://www.cs.uaf.edu/courses/cs441/notes/sse-avx/](https://www.cs.uaf.edu/courses/cs441/notes/sse-avx/)
73. std::atomic_flag \- CPlusPlus, accessed July 8, 2025, [https://cplusplus.com/reference/atomic/atomic_flag/atomic_flag/](https://cplusplus.com/reference/atomic/atomic_flag/atomic_flag/)
74. std::atomic_flag::test_and_set \- cppreference.com, accessed July 8, 2025, [http://en.cppreference.com/w/cpp/atomic/atomic_flag/test_and_set.html](http://en.cppreference.com/w/cpp/atomic/atomic_flag/test_and_set.html)
75. Tutorials \- JUCE, accessed July 8, 2025, [https://juce.com/learn/tutorials/](https://juce.com/learn/tutorials/)
76. tutorial_audio_processor_graph \- JUCE, accessed July 8, 2025, [https://juce.com/tutorials/tutorial_audio_processor_graph/](https://juce.com/tutorials/tutorial_audio_processor_graph/)
77. AudioProcessorGraph Class Reference \- JUCE: Tags, accessed July 8, 2025, [https://docs.juce.com/master/classAudioProcessorGraph.html](https://docs.juce.com/master/classAudioProcessorGraph.html)
78. JUCE/modules/juce_audio_processors/processors/juce_AudioProcessorGraph.h at master, accessed July 8, 2025, [https://github.com/WeAreROLI/JUCE/blob/master/modules/juce_audio_processors/processors/juce_AudioProcessorGraph.h](https://github.com/WeAreROLI/JUCE/blob/master/modules/juce_audio_processors/processors/juce_AudioProcessorGraph.h)
79. Getting Started With AudioProcessorGraph \- JUCE Forum, accessed July 8, 2025, [https://forum.juce.com/t/getting-started-with-audioprocessorgraph/25618](https://forum.juce.com/t/getting-started-with-audioprocessorgraph/25618)
80. Topological Sort: Detailed Explanation and Implementation in C | Abdul Wahab Junaid, accessed July 8, 2025, [https://awjunaid.com/algorithm/topological-sort-detailed-explanation-and-implementation-in-c/](https://awjunaid.com/algorithm/topological-sort-detailed-explanation-and-implementation-in-c/)
81. How to sort parent nodes before child nodes? \- Topological sort \- Cameron MacLeod, accessed July 8, 2025, [https://www.cameronmacleod.com/blog/topological-sort](https://www.cameronmacleod.com/blog/topological-sort)
82. AudioProcessorGraph and loops with addConnection(). What's really happening?, accessed July 8, 2025, [https://forum.juce.com/t/audioprocessorgraph-and-loops-with-addconnection-whats-really-happening/55433](https://forum.juce.com/t/audioprocessorgraph-and-loops-with-addconnection-whats-really-happening/55433)
83. Surge, accessed July 8, 2025, [https://surge-synthesizer.github.io/](https://surge-synthesizer.github.io/)
84. Synthesizer plug-in (previously released as Vember Audio Surge) \- GitHub, accessed July 8, 2025, [https://github.com/surge-synthesizer/surge](https://github.com/surge-synthesizer/surge)
85. surge-synthesizer/sst-cpputils: Various C++ utilities we have collected for cross product use \- GitHub, accessed July 8, 2025, [https://github.com/surge-synthesizer/sst-cpputils](https://github.com/surge-synthesizer/sst-cpputils)
86. Surge Synthesizer Team \- GitHub, accessed July 8, 2025, [https://github.com/surge-synthesizer](https://github.com/surge-synthesizer)
87. The SST Libraries \- Surge Synth Team, accessed July 8, 2025, [https://surge-synth-team.org/sst-libraries/](https://surge-synth-team.org/sst-libraries/)
