import { useState } from "react";

const ESP32_URL = "http://172.20.10.4/toggle";

export default function LedToggle() {
  const [status, setStatus] = useState<string>("");

  const toggleLED = async () => {
    try {
      const response = await fetch(ESP32_URL);
      const text = await response.text();
      setStatus(text);
    } catch (error) {
      setStatus("Error connecting to ESP32");
    }
  };

  return (
    <div>
      <h1>LED Toggle</h1>
      <button onClick={toggleLED}>Toggle LED</button>
      <p>{status}</p>
    </div>
  );
}
