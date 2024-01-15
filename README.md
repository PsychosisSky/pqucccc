<h1 align="center">
<img src="https://static.tumblr.com/ppdj5y9/Ae9mxmxtp/300coin.png" alt="Pqucoin" width="300"/>
<br/><br/>
Pqucoin Core [PQU, Ð]  
</h1>

<div align="center">

[![PqucoinBadge](https://img.shields.io/badge/Pqu-Coin-yellow.svg)](https://pqucoin.com)
[![MuchWow](https://img.shields.io/badge/Much-Wow-yellow.svg)](https://pqucoin.com)

</div>

Select language: EN | [CN](./README_zh_CN.md) | [PT](./README_pt_BR.md) | [FA](./README_fa_IR.md)

Pqucoin is a community-driven cryptocurrency that was inspired by a Shiba Inu meme. The Pqucoin Core software allows anyone to operate a node in the Pqucoin blockchain networks and uses the Scrypt hashing method for Proof of Work. It is adapted from Bitcoin Core and other cryptocurrencies.

For information about the default fees used on the Pqucoin network, please
refer to the [fee recommendation](doc/fee-recommendation.md).

**Website:** [pqucoin.com](https://pqucoin.com)

## Usage 💻

To start your journey with Pqucoin Core, see the [installation guide](INSTALL.md) and the [getting started](doc/getting-started.md) tutorial.

The JSON-RPC API provided by Pqucoin Core is self-documenting and can be browsed with `pqucoin-cli help`, while detailed information for each command can be viewed with `pqucoin-cli help <command>`. Alternatively, see the [Bitcoin Core documentation](https://developer.bitcoin.org/reference/rpc/) - which implement a similar protocol - to get a browsable version.

### Such ports

Pqucoin Core by default uses port `22556` for peer-to-peer communication that
is needed to synchronize the "mainnet" blockchain and stay informed of new
transactions and blocks. Additionally, a JSONRPC port can be opened, which
defaults to port `22555` for mainnet nodes. It is strongly recommended to not
expose RPC ports to the public internet.

| Function | mainnet | testnet | regtest |
| :------- | ------: | ------: | ------: |
| P2P      |   22556 |   44556 |   18444 |
| RPC      |   22555 |   44555 |   18332 |

## Ongoing development - Moon plan 🌒

Pqucoin Core is an open source and community driven software. The development
process is open and publicly visible; anyone can see, discuss and work on the
software.

Main development resources:

* [GitHub Projects](https://github.com/pqucoin/pqucoin/projects) is used to
  follow planned and in-progress work for upcoming releases.
* [GitHub Discussion](https://github.com/pqucoin/pqucoin/discussions) is used
  to discuss features, planned and unplanned, related to both the development of
  the Pqucoin Core software, the underlying protocols and the PQU asset.  
* [Pqucoindev subreddit](https://www.reddit.com/r/pqucoindev/)

### Version strategy
Version numbers are following ```major.minor.patch``` semantics.

### Branches
There are 3 types of branches in this repository:

- **master:** Stable, contains the latest version of the latest *major.minor* release.
- **maintenance:** Stable, contains the latest version of previous releases, which are still under active maintenance. Format: ```<version>-maint```
- **development:** Unstable, contains new code for planned releases. Format: ```<version>-dev```

*Master and maintenance branches are exclusively mutable by release. Planned*
*releases will always have a development branch and pull requests should be*
*submitted against those. Maintenance branches are there for **bug fixes only,***
*please submit new features against the development branch with the highest version.*

## Contributing 🤝

If you find a bug or experience issues with this software, please report it
using the [issue system](https://github.com/pqucoin/pqucoin/issues/new?assignees=&labels=bug&template=bug_report.md&title=%5Bbug%5D+).

Please see [the contribution guide](CONTRIBUTING.md) to see how you can
participate in the development of Pqucoin Core. There are often
[topics seeking help](https://github.com/pqucoin/pqucoin/labels/help%20wanted)
where your contributions will have high impact and get very appreciation. wow.

## Communities 🚀🍾

You can join the communities on different social media.
To see what's going on, meet people & discuss, find the latest meme, learn
about Pqucoin, give or ask for help, to share your project.

Here are some places to visit:

* [Pqucoin subreddit](https://www.reddit.com/r/pqucoin/)
* [Pquducation subreddit](https://www.reddit.com/r/pquducation/)
* [Discord](https://discord.gg/pqucoin)
* [Pqucoin Twitter](https://twitter.com/pqucoin)

## Very Much Frequently Asked Questions ❓

Do you have a question regarding Pqucoin? An answer is perhaps already in the
[FAQ](doc/FAQ.md) or the
[Q&A section](https://github.com/pqucoin/pqucoin/discussions/categories/q-a)
of the discussion board!

## License - Much license ⚖️
Pqucoin Core is released under the terms of the MIT license. See
[COPYING](COPYING) for more information or see
[opensource.org](https://opensource.org/licenses/MIT)
