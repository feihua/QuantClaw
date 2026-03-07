# QuantClaw Official Website Setup

This guide explains how to set up and deploy the QuantClaw official website at **quantclaw.github.io**.

## Quick Start

### 1. Initial Setup

```bash
# Navigate to website directory
cd website

# Install dependencies
npm install

# Start development server
npm run docs:dev

# Open browser to http://localhost:5173
```

### 2. Build the Website

```bash
# Build for production
npm run docs:build

# Preview built site
npm run docs:preview
```

## 📋 Pre-Deployment Checklist

Before deploying, complete these steps:

### ✅ Configure Domain & Repository

1. **In GitHub Repository Settings:**
   - Go to: Settings → Pages
   - Source: Deploy from a branch
   - Branch: `main` (or `develop` for staging)
   - Folder: `/` (root)
   - Custom domain: Leave empty (will be auto-configured for GitHub Pages)

2. **GitHub Pages URL:**
   - Production: `https://quantclaw.github.io`
   - The domain must exactly match: `quantclaw.github.io`

### ✅ Configure Deployment Workflow

The CI/CD pipeline is already configured in `.github/workflows/deploy-website.yml`:

- Triggers on `push` to `main` and `develop` branches
- Automatically builds website with VitePress
- Deploys to GitHub Pages
- No additional configuration needed

### ✅ Create GitHub Organization

If using an organization (recommended):

1. Create GitHub organization: `QuantClaw`
2. Create repository: `quantclaw.github.io`
3. GitHub Pages will automatically deploy to `https://quantclaw.github.io`

### ✅ Generate Favicon

```bash
cd website/public

# Option 1: Online tool (https://icoconvert.com)
# Upload logo.svg and download favicon.ico

# Option 2: ImageMagick
convert -background none logo.svg -define icon:auto-resize=64,48,32,16 favicon.ico

# Option 3: Using GraphicsMagick
gm convert logo.svg favicon.ico
```

Place the generated `favicon.ico` in `website/public/`.

## 🚀 Deployment

### Automatic Deployment (Recommended)

The website automatically deploys via GitHub Actions when you push changes:

```bash
# Make changes to website files
cd website
vim guide/getting-started.md

# Commit and push
git add .
git commit -m "Update documentation"
git push origin main  # or develop
```

The workflow in `.github/workflows/deploy-website.yml` will:
1. Trigger automatically
2. Install dependencies
3. Build the website
4. Deploy to GitHub Pages

**Check deployment status:**
- Go to: GitHub Repo → Actions
- Find the "Deploy Website" workflow
- Check the deployment status

### Manual Deployment

If needed, you can manually deploy:

```bash
# Build the site
cd website
npm run docs:build

# The output in .vitepress/dist/ can be deployed to any static host
# Or use GitHub's manual deployment process
```

## 📊 Configuration Details

### VitePress Config (`website/.vitepress/config.ts`)

Key settings for GitHub Pages:

```typescript
export default defineConfig({
  base: '/',  // Root path for quantclaw.github.io
  title: 'QuantClaw',
  description: 'High-performance C++17 AI agent framework',
  // ...
})
```

For custom domain, you can adjust `base` if needed.

### GitHub Actions Workflow

File: `.github/workflows/deploy-website.yml`

**Key points:**
- Runs on push to `main` and `develop`
- Triggers on changes to `website/` directory
- Uses Node.js 18
- Deploys using official `actions/deploy-pages@v4`

## 📝 Updating Content

### Add New Page

1. Create markdown file in `website/guide/`:
   ```bash
   touch website/guide/new-topic.md
   ```

2. Edit and add content

3. Update sidebar in `website/.vitepress/config.ts`:
   ```typescript
   {
     text: 'New Topic',
     link: '/guide/new-topic'
   }
   ```

4. Commit and push (auto-deploys)

### Customize Appearance

Edit `website/.vitepress/theme/custom.css`:

```css
:root {
  --vp-c-brand: #2d3748;           /* Primary color */
  --vp-c-accent: #06b6d4;          /* Accent color */
  /* ... more customizations ... */
}
```

## 🔗 Useful Links

- **Website**: https://quantclaw.github.io
- **GitHub Repository**: https://github.com/QuantClaw/quantclaw
- **VitePress Docs**: https://vitepress.dev/
- **GitHub Pages Docs**: https://docs.github.com/en/pages

## 🐛 Troubleshooting

### Website Not Updating After Push

1. Check GitHub Actions:
   - Go to: Repository → Actions
   - Find latest "Deploy Website" workflow
   - Check if it passed or failed

2. Clear browser cache:
   - Hard refresh: `Ctrl+Shift+R` (Windows/Linux) or `Cmd+Shift+R` (Mac)

3. Check `base` path in config:
   - Should be `/` for `quantclaw.github.io`
   - For subdomain, use `/subdomain/`

### Build Errors

```bash
# Clean and rebuild
cd website
rm -rf node_modules package-lock.json .vitepress/.temp
npm install
npm run docs:build
```

### Port Already in Use

```bash
npm run docs:dev -- --port 3000
```

### Favicon Not Showing

1. Verify `favicon.ico` exists in `website/public/`
2. Hard refresh browser cache
3. Check browser console for errors

## 📚 Directory Structure

```
QuantClaw/
├── website/                          # Website source
│   ├── .vitepress/
│   │   ├── config.ts                # VitePress configuration
│   │   ├── theme/
│   │   │   ├── index.ts
│   │   │   └── custom.css
│   │   └── dist/                    # Built output (generated)
│   ├── guide/                       # Documentation pages
│   │   ├── getting-started.md
│   │   ├── installation.md
│   │   ├── features.md
│   │   ├── architecture.md
│   │   ├── configuration.md
│   │   ├── plugins.md
│   │   ├── cli-reference.md
│   │   ├── building.md
│   │   ├── contributing.md
│   │   └── documentation.md
│   ├── public/                      # Static assets
│   │   ├── logo.svg
│   │   └── favicon.ico              # ⚠️ Needs to be generated
│   ├── index.md                     # Homepage
│   ├── package.json
│   └── README.md
├── .github/
│   └── workflows/
│       └── deploy-website.yml       # Auto-deployment workflow
└── WEBSITE_SETUP.md                # This file
```

## 🚀 Next Steps

1. ✅ Generate `favicon.ico`
2. ✅ Verify GitHub Actions workflow
3. ✅ Test local development
4. ✅ Push to repository
5. ✅ Verify GitHub Pages deployment

## 📞 Support

For issues with:
- **VitePress**: Check [VitePress Documentation](https://vitepress.dev/)
- **GitHub Pages**: Check [GitHub Pages Help](https://docs.github.com/en/pages)
- **QuantClaw**: Open issue on [GitHub](https://github.com/QuantClaw/quantclaw/issues)

---

**Website is ready to deploy! 🎉**
