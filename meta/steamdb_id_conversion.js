// Strings for testing
var steam_id_strings =
[
    `[U:1:22202]`,    // GabeN's account
    `[U:1:169802]`,   // Robin's account
    `[U:1:324394636]` // My account
]

for (let i = 0; i < steam_id_strings.length; ++i)
{
    console.log(`ID #${i}:`)

    // Original website code excerpt
    {
        let input_output_string = steam_id_strings[i].slice(), match = 0, a = 0, n = 0;

        (n = input_output_string.match(/^STEAM_[0-5]:(?<universe>[0-1]):(?<id>[0-9]+)$/)) ? match = parseInt(n.groups.id, 10) << 1 | parseInt(n.groups.universe, 10): (n = input_output_string.match(/^\[U:1:(?<id>[0-9]+)\]$/)) || (n = input_output_string.match(/tradeoffer\/new\/\?partner=(?<id>[0-9]+)/)) ? match = parseInt(n.groups.id, 10) : (n = input_output_string.match(/^(?:steam:)?([a-fA-F0-9]{15})$/)) ? input_output_string = BigInt("0x" + n[1]).toString() : (n = input_output_string.match(/(76561[0-9]{12})/)) && (input_output_string = n[1]), match > 0 && (input_output_string = (1n << 56n | 1n << 52n | 1n << 32n | BigInt(match)).toString())

        console.log(`      Via original workhorse code: ${input_output_string}`)
    }

    // Simplified workhorse code
    {
        let input_output_string = steam_id_strings[i].slice(), value = 0, match = 0, n = 0;

        if (match = input_output_string.match(/^STEAM_[0-5]:(?<universe>[0-1]):(?<id>[0-9]+)$/))
        {
            value = parseInt(match.groups.id, 10) << 1 | parseInt(match.groups.universe, 10)
        }
        else
        {
            if ((match = input_output_string.match(/^\[U:1:(?<id>[0-9]+)\]$/)) || (match = input_output_string.match(/tradeoffer\/new\/\?partner=(?<id>[0-9]+)/)))
            {
                value = parseInt(match.groups.id, 10)
            }
            else
            {
                match = input_output_string.match(/^(?:steam:)?([a-fA-F0-9]{15})$/)
            }
        }

        if (match)
        {
            input_output_string = BigInt("0x" + match[1]).toString()
        }
        else
        {
            (match = input_output_string.match(/(76561[0-9]{12})/))
        }

        if (match)
        {
            (input_output_string = match[1])
        }

        if (value > 0)
        {
            (input_output_string = (1n << 56n | 1n << 52n | 1n << 32n | BigInt(value)).toString())
        }

        console.log(`    Via simplified workhorse code: ${input_output_string}`)
    }

    // "My" simplified code, specialized for STEAMID3's *only*
    {
        const output = (76561197960265728n | BigInt(parseInt(steam_id_strings[i].slice(5, steam_id_strings[i].length - 1))))

        console.log(`           Via my simplified code: ${output}`)
    }
}

/*
Original code:
-----------------------------------------------------------------------

"use strict";
(function() {
  "use strict";
  const i = window.SteamDB;
  document.getElementById("js-lookup-profile").addEventListener("submit", function(t) {
    const e = document.querySelector("#js-lookup-input"),
      o = document.querySelector("#js-lookup-submit");
    let r = e.value.trim(),
      n;
    if (o.classList.contains("loading")) {
      t.preventDefault();
      return
    }(n = r.match(/\/(?:id|profiles?)\/(?<id>[\d\w\-:[\]]+)/)) && (r = n.groups.id);
    const d = /^76561[0-9]{12}$/;
    if (!d.test(r)) {
      let a = 0;
      (n = r.match(/^STEAM_[0-5]:(?<universe>[0-1]):(?<id>[0-9]+)$/)) ? a = parseInt(n.groups.id, 10) << 1 | parseInt(n.groups.universe, 10): (n = r.match(/^\[U:1:(?<id>[0-9]+)\]$/)) || (n = r.match(/tradeoffer\/new\/\?partner=(?<id>[0-9]+)/)) ? a = parseInt(n.groups.id, 10) : (n = r.match(/^(?:steam:)?([a-fA-F0-9]{15})$/)) ? r = BigInt("0x" + n[1]).toString() : (n = r.match(/(76561[0-9]{12})/)) && (r = n[1]), a > 0 && (r = (1n << 56n | 1n << 52n | 1n << 32n | BigInt(a)).toString())
    }
    if (r.includes(" ")) {
      t.preventDefault(), e.setCustomValidity("SteamID cannot contain spaces, you are probably doing it wrong."), e.reportValidity(), e.addEventListener("input", () => {
        e.setCustomValidity("")
      }, {
        once: !0
      });
      return
    }
    e.readOnly = !0, e.value = r, o.classList.add("loading"), o.textContent = "Hunting ducks\u2026";
    const m = document.querySelector(".calculator-takeover-input");
    if (m.classList.add("calculator-takeover-loading"), window.addEventListener("pageshow", a => {
        a.persisted && (m.classList.remove("calculator-takeover-loading"), e.readOnly = !1, o.classList.remove("loading"), o.textContent = "Let's go")
      }, {
        once: !0
      }), d.test(r)) {
      t.preventDefault();
      const u = document.querySelector("#js-input-currency").value;
      window.location.href = `/calculator/${r}/?cc=${u}`
    }
  }), document.querySelectorAll(".steamid-copy").forEach(t => {
    const e = t.getAttribute("aria-label");
    t.addEventListener("click", function(o) {
      o.preventDefault();
      const r = this.previousElementSibling.textContent;
      navigator.clipboard.writeText(r).then(() => {
        t.setAttribute("aria-label", "Copied!"), setTimeout(() => t.setAttribute("aria-label", e), 1e3)
      }, n => {
        i.Alert(`Failed to copy: ${n}`)
      })
    })
  });
  const h = t => {
      if (t.tagName === "SPAN" && !t.checkVisibility()) {
        t.textContent = "";
        return
      }
      for (const e of t.children) h(e)
    },
    g = (t, e) => {
      const o = document.getElementById(t);
      if (!o) return "";
      e ? o.parentElement.hidden = !1 : o.hidden = !1, h(o);
      const r = o.textContent.replace(/\s+/g, " ").trim();
      return e ? o.parentElement.hidden = !0 : o.hidden = !0, r
    };
  document.querySelector(".calculator-share")?.addEventListener("click", function(t) {
    if (t.preventDefault(), !("share" in navigator)) {
      i.Alert("Your browser does not support native share. Use the copy buttons.");
      return
    }
    const e = g("js-share-text-no-url", !0);
    navigator.share({
      url: document.querySelector('link[rel="canonical"]').href,
      text: e
    }).catch(() => {})
  }), document.querySelectorAll(".calculator-copy").forEach(t => {
    t.addEventListener("click", function(e) {
      e.preventDefault();
      const o = g(t.dataset.target);
      navigator.clipboard.writeText(o).then(() => {
        t.classList.add("btn-primary"), setTimeout(() => t.classList.remove("btn-primary"), 1e3)
      }, r => {
        i.Alert(`Failed to copy: ${r}`)
      })
    })
  }), document.querySelector(".js-calculator-toggle-copy")?.addEventListener("click", function(t) {
    t.preventDefault();
    const e = document.querySelector(".calculator-copy-buttons");
    e.hidden = !e.hidden
  });
  const c = document.querySelector(".roulette-game-data");
  if (c) {
    const t = i.CalculatorRandomGames || [];
    delete i.CalculatorRandomGames;
    let e = !1,
      o = null;
    const r = document.querySelector(".roulette-play"),
      n = document.querySelector(".roulette-reroll"),
      d = () => {
        const a = c.querySelector("video");
        a && a.remove(), c.replaceChildren()
      },
      m = () => {
        if (t.length === 0) {
          document.querySelector(".roulette-buttons").hidden = !0, d(), c.textContent = "You have run out of games to play.";
          return
        }
        const a = Math.floor(Math.random() * t.length),
          u = t[a];
        t.splice(a, 1), r.href = `steam://run/${u}/`, e = !0, n.disabled = !0, fetch(`/api/RenderAppHover/?appid=${u}`, {
          headers: {
            Accept: "text/html",
            "X-Requested-With": "XMLHttpRequest"
          }
        }).then(s => {
          if (e = !1, setTimeout(() => {
              n.disabled = !1
            }, 500), !s.ok) {
            const f = new Error(`HTTP ${s.status}`);
            throw f.name = "ServerError", f
          }
          return s.text()
        }).then(s => {
          d(), c.insertAdjacentHTML("beforeend", s);
          const f = c.firstElementChild,
            S = f.querySelector(".js-open-screenshot-viewer");
          S && S.classList.remove("js-open-screenshot-viewer");
          const v = f.querySelector(".hover_video");
          if (v) {
            const l = document.createElement("video");
            l.controls = !0, l.autoplay = o || !1, l.muted = !0, l.loop = !0;
            const E = document.body.dataset.videoCdn,
              b = JSON.parse(v.dataset.microtrailer);
            for (const [L, q] of Object.entries(b.video)) {
              const y = document.createElement("source");
              y.type = L, y.src = `${E}store_trailers/${q}?t=${b.time}`, l.append(y)
            }
            l.addEventListener("pause", () => {
              o = !1
            }), l.addEventListener("play", () => {
              o = !0
            }), v.appendChild(l)
          }
        }).catch(s => {
          if (d(), c.textContent = `Failed to load game data for app ${u}: ${s}`, !(s.name === "AbortError" || s.name === "ServerError")) throw s
        })
      };
    i.LazyLoad(c, () => {
      m()
    }), n.addEventListener("click", function(a) {
      e || (o ??= !0, m(), a.preventDefault())
    })
  }
  const p = document.getElementById("table-apps");
  if (p?.querySelector("tfoot")) {
    const t = document.body.dataset.storeAssets;
    i.BindHover && i.BindHover(p), $(p).DataTable({
      columnDefs: [{
        orderSequence: ["desc", "asc"],
        targets: ["_all"]
      }],
      initComplete() {
        p.querySelector("tfoot")?.remove(), p.querySelector("tbody").hidden = !1
      },
      rowCallback(e) {
        if (e.dataset.loaded) return;
        e.dataset.loaded = "true";
        const o = e.querySelector(".applogo img");
        o.addEventListener("error", function() {
          this.src = "/static/img/applogo.svg"
        }, {
          once: !0
        }), o.src = `${t}apps/${e.dataset.appid}/${e.dataset.capsule}`
      }
    })
  }
})();

P.S: The Javascript language and obfuscation at large MUST be destroyed for the sake of humanity's continued survival.
*/
