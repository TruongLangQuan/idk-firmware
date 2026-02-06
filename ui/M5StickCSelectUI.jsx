import React, { useState } from "react";
import { motion } from "framer-motion";

// Canvas size: 240 x 135 (landscape-left)
// Simple firmware-style menu UI for M5StickC Plus2

const options = [
  "Wifi",
  "Clock",
  "Img",
  "Gif",
  "Games",
  "Test",
  "IR",
  "Files",
  "Setting",
];

export default function M5StickCSelectUI() {
  const [selected, setSelected] = useState(0);

  const handleUp = () => {
    setSelected((prev) => (prev - 1 + options.length) % options.length);
  };

  const handleDown = () => {
    setSelected((prev) => (prev + 1) % options.length);
  };

  return (
    <div className="flex items-center justify-center min-h-screen bg-neutral-100">
      <div
        className="relative bg-black border border-neutral-500 overflow-hidden"
        style={{ width: 240, height: 135 }}
      >
        <div className="text-[10px] text-white px-2 py-1 bg-[#1a1a1a] border-b border-neutral-700 font-mono">
          Bat: 100%
        </div>

        <div className="flex flex-col px-1 py-1 gap-[2px] overflow-hidden">
          {options.map((item, index) => (
            <motion.div
              key={item}
              animate={{
                backgroundColor: index === selected ? "#1f1e1e" : "#000000",
                color: "#ffffff",
              }}
              transition={{ duration: 0.15 }}
              className="text-[11px] px-2 py-[2px] w-full font-mono"
            >
              {item}
            </motion.div>
          ))}
        </div>

        {/* Optional controls (for demo) */}
        <div className="absolute bottom-1 right-1 flex gap-1 text-[8px] text-neutral-400 font-mono">
          <button onClick={handleUp} className="px-1 border border-neutral-600">Prev</button>
          <button onClick={handleDown} className="px-1 border border-neutral-600">Next</button>
        </div>
      </div>
    </div>
  );
}
