# Termux → GitHub → Geode build

> Android is enough to upload the source. The actual Geode compilation happens on GitHub Actions.

## 1. Install Termux

Use the F-Droid build of Termux. Then:

```bash
pkg update -y
pkg install git gh -y
```

## 2. Authenticate GitHub

```bash
gh auth login
```

Choose GitHub.com, HTTPS, and complete the browser/device-code login.

## 3. Create the repository

On GitHub create an empty repository named `frame-window-counter`.

In Termux, enter your project folder:

```bash
cd ~/frame-window-counter
```

Then:

```bash
git init
git add .
git commit -m "Initial commit"
git branch -M main
git remote add origin https://github.com/ВАШ_ЮЗЕРНЕЙМ/frame-window-counter.git
git push -u origin main
```

## 4. Start the build

Open the repository on GitHub → **Actions** → **Build Geode Mod** → **Run workflow**.

The same workflow also runs automatically after a push.

## 5. Get the `.geode`

Wait for all platform jobs to pass. Open the `package` job and download the artifact named **frame-window-counter**. It contains the combined `.geode` package.

## 6. Install

Copy the `.geode` into the Geometry Dash Geode `mods` directory on the device/PC where Geometry Dash is installed, then launch the game.

## 7. Manual frame-window workflow

For live captures:

1. Start a level.
2. The mod records press/release events and assigns a 240 Hz frame/time coordinate.
3. Set **Manual Window** to `1..10` in Geode settings.
4. Press **Manual Stamp Key** after selecting the measured FW for the latest unknown action.
5. Imported `.gdr/.gdr2/.slc` files also start with unknown FW values; this is expected.

For bulk work, prepare a NaNDL JSON or `.fwc` file with actual `frameWindow` values and set **Import Path**, then press **Import Key**.
