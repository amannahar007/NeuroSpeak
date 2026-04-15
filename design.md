# NeuroSpeak Web Dashboard - Design System

This document outlines the visual identity and aesthetic principles for the NeuroSpeak Web Dashboard. The dashboard serves as the control center and live monitoring interface for the ESP32-C6 local ML EMG system. Given the nature of the project, the interface must exude a sense of "premium sci-fi", high-tech realism, and responsive immediacy.

## 1. Aesthetic Vision
The overarching theme is **State-of-the-Art Neural Tech**. It should feel less like a standard web page and more like a dedicated machine terminal dashboard.
- **Vibe:** Dark, focused, vibrant highlights over deep structural elements, fluid motion, precision.
- **Glassmorphism:** Subdued translucent panels with varying opacities to provide depth over deep background tones.

## 2. Color Palette
We avoid generic reds and greens. We utilize fine-tuned HSL values to provide precise saturation and luminosity.

*   **Background (Void):** `#06080F` (Very dark blue/black - acts as the infinite canvas)
*   **Surface Level 1 (Card Base):** `rgba(18, 22, 38, 0.7)` (Translucent deep navy for primary panels)
*   **Surface Level 2 (Interactive/Hover):** `rgba(28, 35, 60, 0.9)`
*   **Primary Accent (Neural Cyan):** `#00e5ff` (Used for active connections, primary graphs, key buttons)
*   **Secondary Accent (Pulse Purple):** `#b026ff` (Used for ML model activity, training actions)
*   **Status: Live/Success:** `#00ff9d` (Bright spring green)
*   **Status: Recording (Hot):** `#ff2a5f` (Vibrant hot pink/red)
*   **Status: Idle/Disconnected:** `#4a5568` / `#ffb020` (Muted gray or warning orange depending on state)
*   **Text (Primary):** `#e2e8f0` (Soft white/gray for readability)
*   **Text (Secondary/Muted):** `#718096`

## 3. Typography
We use modern, technical, and easily scannable sans-serif typefaces.
*   **Primary Typeface:** `Inter` (Google Fonts) - Clean, neutral, perfect for UI.
*   **Monospace Typeface:** `JetBrains Mono` or `Fira Code` - For logs, timestamps, and raw data outputs.
*   **Hierarchy:**
    *   `h1` / `h2` (Headers): Bold (700), slight letter spacing, potentially incorporating accent gradients.
    *   `UI Labels`: Uppercase, bold, 12px, letter-spacing 1px, `Text (Secondary)`.
    *   `Data Values`: Large, Monospace, crisp.

## 4. UI Elements & Components

### Cards & Panels
- **Border:** `1px solid rgba(255, 255, 255, 0.05)`
- **Border Radius:** `16px` for outer panels, `8px` for inner components.
- **Backdrop Filter:** `blur(12px)` for glassmorphism.
- **Shadow:** Deep and soft, e.g., `0 20px 40px rgba(0, 0, 0, 0.5)`.

### Buttons
- **Shape:** Pill-shaped or slightly rounded rectangles (`8px` radius).
- **Default State:** Semi-transparent accent background (e.g., `rgba(0, 229, 255, 0.1)`) with `1px` solid border.
- **Hover State:** Dynamic glow (`box-shadow: 0 0 15px rgba(0, 229, 255, 0.4)`), slightly lighter background, transition all `0.2s ease-out`.
- **Recording Button (Active):** High intensity pulsing animation using keyframes to indicate "recording".

### Typography & Gradients
- Use background-clip text for primary headings (Cyan to Purple gradient) to make them pop.
- Emphasize numbers and real-time changing data with monospace fonts to prevent layout shifting.

## 5. Micro-Animations & Interactions
- **Live Dot:** A small SVG circle next to connection statuses with a 2-second breathing animation (`transform: scale(1) to scale(1.2)` and opacity fading).
- **Panel Entrances:** Slight fade-in and slide-up (`transform: translateY(10px)`) staggered on page load for a staggered, mechanical boot-up feel.
- **Hover Transitions:** All interactive elements must react instantly but smoothly (`0.15s` duration).

## 6. Layout Structure
A fully responsive CSS Grid layout focusing on desktop but adjusting down gracefully.
- **Top Bar:** Branding, overall connection status summary, toggle settings.
- **Main Content (Grid):**
    - **Left Column:** Live Detection readout (large), Label Assignment inputs, Recording Status logic.
    - **Middle Column:** Debug Views (Real-time EMG live scope using Canvas or WebGL).
    - **Right Column:** Technical Event Log (auto-scrolling).

## 7. Development Guidelines
- Use Vanilla CSS for complete control over specifics.
- Avoid bulky frameworks unless absolutely necessary; CSS Variables (`--var-name`) will be used to manage themes centrally.
- No generic place-holders; the UI must look production-ready immediately.
